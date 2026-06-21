# MazeSurge

C++ と DirectX 11 を用いて、ゲームエンジンを一切使わずにゼロから作った3Dトップダウン迷路ゲームです。
ゲームエンジンに頼らず迷路生成・衝突判定・弾丸システム・シーン管理などのゲームロジックと、
レンダリングパイプラインを自前で実装することで、3Dゲーム開発に必要な数学知識・設計知識・レンダリング知識を習得することを目的としています。

![Demo movie](docs/demo.gif)

## 📅 開発期間 (Development period)
2026年5月~2026年6月

## 🛠 技術スタック (Tech Stack)

| 項目 | 内容 |
|---|---|
| **Graphics API** | DirectX 11 (Direct3D 11, DXGI) |
| **Shader** | HLSL (Shader Model 5.0) |
| **Language** | C++17 |
| **Math** | DirectXMath |
| **2D UI** | SpriteBatch / SpriteFont (DirectXTK) |
| **Build** | Visual Studio 2026 |
| **Platform** | Windows |

## 🎮 ゲーム概要 (Game Overview)

制限時間 **5分** 以内にランダム生成された迷路を探索し、ゴールを目指すトップダウン視点の3Dシューティング迷路ゲームです。

- **移動:** WASD でプレイヤーを移動
- **射撃:** 左クリックでマウスカーソル方向に弾丸を発射
- **チェックポイント:** 迷路内に散在するチェックポイントブロックを回収（スコアに影響）
- **ゴール:** ゴールブロックに到達するとゲームクリア
- **敵:** 迷路外周からスポーンし、プレイヤーを追跡。接触で HP が1減少
- **HP　& TIME:** HPの初期値は5。制限時間は5分。HPが0になるか制限時間を超過するとゲームオーバー
- **ランク:** クリア時の残り時間とチェックポイント取得率の平均でS/A/B/C/Dを算出

## 🚀 技術的なこだわり (Technical Highlights)

### 1. 迷路自動生成（穴掘り法）

穴掘り法でランダムな迷路をゲーム起動ごとに自動生成しています。

```cpp
// Dungeon.cpp — スタックを使ったイテレーティブ実装
while (!stk.empty())
{
    auto [nowI, nowJ] = stk.top();
    int startIndex = rand() % 4;
    bool all = true;
    for (int k = 0; k < 4; k++)
    {
        int newI = nowI + di[order[startIndex + k]] * 2;
        int newJ = nowJ + dj[order[startIndex + k]] * 2;
        if (m_grid[newI][newJ] == CellType::WALL)
        {
            m_grid[nowI + ...][nowJ + ...] = CellType::FLOOR;
            stk.push({newI, newJ});
            all = false; break;
        }
    }
    if (all) stk.pop();
}
```

- 再帰ではなくスタックを使ったイテレーティブ実装でスタックオーバーフローを回避
- 方向をランダムオフセット付きで選択することで毎回異なる迷路形状を生成

### 2. BFS によるゴール・チェックポイント配置

迷路生成後に BFS でスタートから全セルまでの距離を計測し、最も遠いセルをゴールに、行き止まりをチェックポイントに自動配置しています。

```cpp
// Dungeon.cpp — BFS で最遠セルを探索
while (!que.empty())
{
    auto [nowI, nowJ, nowDist] = que.front(); que.pop();
    distL1[nowI][nowJ] = nowDist;
    if (maxDist < nowDist) { maxDist = nowDist; goalGrid = {nowI, nowJ}; }
    // ...隣接 FLOOR セルをキューに追加...
}
m_grid[goalGrid.first][goalGrid.second] = CellType::GOAL;
```

- ゴールが常にスタートから最も遠い位置に配置されるため、どんな迷路でも適切な難易度を保証
- チェックポイントはスタート付近を避けるため、BFS 訪問順の先頭2要素の行き止まりを除外

### 3. AABB 衝突判定と軸分離移動（壁スライド）

プレイヤーの X 軸と Z 軸の移動を独立に解決することで、壁に対するスライド移動を実現しています。

```cpp
// Player.cpp — X→Z の順に独立した壁衝突解決
float newX = m_position.x + velocity.x * m_speed * deltaTime;
if (g_dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) || ...) newX = m_position.x;

float newZ = m_position.z + velocity.z * m_speed * deltaTime;
if (g_dungeon.IsWall(newX - HALF_SIZE, newZ - HALF_SIZE) || ...) newZ = m_position.z;
```

```cpp
// Collision.cpp — AABB（2D）衝突判定
bool Collision::checkAABB(const BBOX &box1, const BBOX &box2)
{
    return box1.minX <= box2.maxX && box1.maxX >= box2.minX &&
           box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ;
}
```

- バウンディングボックスの4隅すべてをチェックして隅へのめり込みを防止
- Z 判定には確定後の `newX` を使うことで対角コーナーへの引っ掛かりを解消

### 4. オブジェクトプールパターン（弾丸・敵）

弾丸と敵を毎フレーム `new`/`delete` せず、固定サイズのプール配列を使い回す設計で動的メモリアロケーションを排除しています。

```cpp
// EnemyManager.cpp — プール内の非活性スロットを探して再利用
void EnemyManager::SpawnEnemy(Dungeon &dungeon)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
        {
            m_pool[i].active = true;
            // ... 初期化処理 ...
            return;
        }
    }
}
```

- `active` フラグで活性/非活性を管理し、非活性スロットをスポーン時に再利用
- 弾丸プール（50発）・敵プール（50体）を分離して独立管理

### 5. スクリーン→ワールド変換による射撃照準

マウスのスクリーン座標を `ScreenToWorldOnPlane` でワールド空間に逆変換し、プレイヤーからカーソル方向へ弾丸を発射しています。

```cpp
// Camera.cpp — ニア/ファークリップ面上の点を逆変換してレイ方向を算出
XMFLOAT3 Camera::ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const
{
    XMVECTOR nWorldPos = XMVector3Unproject(nViewPos, ...); // ニアクリップ面
    XMVECTOR fWorldPos = XMVector3Unproject(fViewPos, ...); // ファークリップ面
    XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(fWorldPos, nWorldPos));
    // t = (planeY - origin.y) / dir.y でプレイヤー平面との交点を計算
}
```

- 描画時のローカル→クリップ→スクリーン変換の逆過程（スクリーン→ワールド）を実行
- 水平面とのパラメトリック交差判定でカーソルのワールド座標を算出

### 6. プレイヤー追従トップダウンカメラ

カメラをプレイヤー頭上に固定オフセットで配置し、注視点をプレイヤー位置に追従させる俯瞰カメラを実装しています。

```cpp
// Camera.cpp — プレイヤー位置から固定オフセットでカメラ位置を算出
void Camera::Update(const XMFLOAT3 &targetPos)
{
    m_target   = targetPos;
    m_position = {targetPos.x, targetPos.y + m_offsetY, targetPos.z + m_offsetZ};
}
```

- Y/Z オフセットにより見下ろし角を固定し、常にプレイヤーを画面中央に捉え続ける
- ビュー行列は `XMMatrixLookAtLH(position, target, up)` で毎フレーム再計算

### 7. シーン管理とスコアリング

タイトルシーンとゲームシーンを `Scene` 基底クラスで抽象化し、`GameState` 列挙型で状態遷移を管理しています。

```cpp
// GameScene.cpp — 残り時間比率とチェックポイント取得率の平均でランクを算出
float timeScore       = (m_timeLimit - m_elapsedTime) / m_timeLimit;
float checkPointScore = static_cast<float>(m_getCheckPoint) / static_cast<float>(m_dungeon.GetCheckPointNum());
float totalScore      = (timeScore + checkPointScore) / 2.f;
```

- ゲームオーバー/クリア条件を Update 内で集中管理し、状態に応じて Draw を切り替え
- スコアは残り時間比率とチェックポイント取得率の均等加重平均で計算

### 8. HLSL フォンシェーディング

ブリン・フォン反射モデルによるアンビエント・ディフューズ・スペキュラ・エミッシブの4成分でキューブを照明しています。

```hlsl
// shaders.hlsl — ハーフベクトルを用いたブリン・フォン近似
float3 V = normalize(cameraPosition - input.worldPos);
float3 H = normalize(L + V);
float spec = pow(max(dot(N, H), 0.0f), shininess);
float3 emissive = baseColor * emissiveIntensity;
float3 finalColor = ambient + diffuse + specular + emissive;
```

- エミッシブ成分により、ライト環境に依存せず各ゲームオブジェクトの色が視認しやすい
- 定数バッファ `b0`（WVP行列・オブジェクト色）と `b1`（ライト情報）の2スロット構成

## 📂 ディレクトリ構成 (Project Structure)

```
MazeSurge/
├── include/MazeSurge/
│   ├── Core/
│   │   ├── Common.h          # 共有インクルード・型定義
│   │   ├── Types.h           # Vertex, ConstantBuffer, BBOX 構造体
│   │   ├── Core.h            # コアヘッダのまとめ
│   │   └── GameConstant.h    # ゲーム全体の定数定義
│   ├── Game/
│   │   ├── Dungeon.h         # 迷路グリッド・生成・描画
│   │   ├── Player.h          # プレイヤー移動・HP管理
│   │   ├── Enemy.h           # 敵データ構造体
│   │   ├── EnemyManager.h    # 敵のスポーン・更新・プール管理
│   │   ├── Projectile.h      # 弾丸データ構造体
│   │   └── ProjectilePool.h  # 弾丸プール・射撃処理
│   ├── Graphics/
│   │   ├── Renderer.h        # D3D11 リソース管理・描画・UI
│   │   ├── Camera.h          # 追従カメラ・スクリーン→ワールド変換
│   │   └── Cube.h            # キューブ頂点・インデックスデータ
│   ├── Scene/
│   │   ├── Scene.h           # シーン基底クラス
│   │   ├── TitleScene.h      # タイトルシーン
│   │   └── GameScene.h       # ゲームシーン（ゲームループ・状態管理）
│   ├── UI/
│   │   └── UIConstant.h      # UI レイアウト定数
│   ├── Utility/
│   │   └── Collision.h       # AABB 衝突判定
│   └── External/
│       └── tiny_obj_loader.h
├── src/
│   ├── main.cpp              # WinMain, WndProc, シーン切り替え
│   ├── Game/
│   │   ├── Dungeon.cpp       # 穴掘り法・BFS・座標変換
│   │   ├── Player.cpp        # WASD移動・壁衝突解決
│   │   ├── Enemy.cpp         # 敵の初期化
│   │   ├── EnemyManager.cpp  # スポーン間隔加速・プレイヤー追尾
│   │   ├── Projectile.cpp    # 弾丸の初期化
│   │   └── ProjectilePool.cpp# 射撃・弾丸更新・壁衝突
│   ├── Graphics/
│   │   ├── Renderer.cpp      # パイプライン初期化・描画・SpriteBatch UI
│   │   └── Camera.cpp        # ビュー/プロジェクション行列・レイキャスト
│   ├── Scene/
│   │   ├── TitleScene.cpp    # タイトル描画・入力待ち
│   │   └── GameScene.cpp     # ゲームロジック・ランク算出
│   └── Utility/
│       └── Collision.cpp     # AABB 判定実装
└── shaders/
    └── shaders.hlsl          # 頂点/ピクセルシェーダー（ブリン・フォン）
```

## 📦 ビルド方法 (Build Instructions)

### 前提条件

- Windows 10 以降
- Visual Studio 2026（C++ デスクトップ開発ワークロード）
- DirectXTK NuGet パッケージ（プロジェクトに設定済み）

### 手順

```bash
# リポジトリのクローン
git clone https://github.com/niajf/MazeSurge.git

# Visual Studio でソリューションを開く
# MiazeSurge.slnx をダブルクリック

# ビルド: Ctrl + Shift + B
# 実行:   F5
```

## 🎯 実装機能一覧 (Features)

- **迷路自動生成:** 穴掘り法によるランダム迷路生成
- **ゴール・CP 配置:** BFS による最遠ゴール配置と行き止まりへのチェックポイント配置
- **WASD 移動:** X/Z 独立衝突解決による壁スライド移動
- **AABB 衝突判定:** プレイヤー・敵・弾丸・壁の2D矩形衝突判定
- **射撃システム:** スクリーン→ワールド逆変換による照準、弾丸プール管理
- **敵 AI:** 円周ランダムスポーン、プレイヤー追尾、時間経過でスポーン間隔が加速
- **オブジェクトプール:** 弾丸50発・敵50体を動的確保なしで使い回し
- **追従カメラ:** プレイヤー頭上固定オフセットによるトップダウン俯瞰カメラ
- **シーン管理:** TitleScene / GameScene を `GameState` 列挙型で状態遷移
- **スコア/ランク:** 残り時間比率とチェックポイント取得率の平均で S/A/B/C/D を算出
- **HUD:** SpriteBatch による残り時間・HP・チェックポイント取得数の 2D UI 表示
- **フォンシェーディング:** HLSL によるアンビエント・ディフューズ・スペキュラ・エミッシブ照明
- **D3D11 パイプライン:** デバイス・スワップチェイン・レンダーターゲット・深度バッファの手動構築
