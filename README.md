# Muzui Mazui Mazey!

C++ と DirectX 11 を用いて、開発した3Dトップダウン迷路シューティングゲームです。
**迷路の自動生成・BFS による敵の経路探索・オブジェクトプーリング** など、ゲームを支えるアルゴリズムとデータ構造を自前で設計・実装することに重点を置いています。
「毎回異なる迷路でも破綻しないレベルデザイン」「敵が何体いても軽い経路探索」「ゲーム中に動的確保をしないメモリ設計」を、アルゴリズムの選択によって実現しました。

![Demo movie](docs/demo.gif)

## 📅 開発期間 (Development period)
- **v1.x:** 2026年5月~2026年6月
- **v2 (develop):** 2026年9月（敵 AI の経路探索化・スポーンアルゴリズム刷新・サウンド・演出強化）

## 🛠 技術スタック (Tech Stack)

| 項目 | 内容 |
|---|---|
| **Language** | C++17 |
| **Graphics API** | DirectX 11 (Direct3D 11, DXGI) |
| **Shader** | HLSL (Shader Model 5.0) |
| **Math** | DirectXMath |
| **Audio** | XAudio2 + dr_mp3（MP3 デコード） |
| **2D UI** | SpriteBatch / SpriteFont (DirectXTK) |
| **Build** | Visual Studio 2026 |
| **Platform** | Windows |

## 🎮 ゲーム概要 (Game Overview)

制限時間 **5分** 以内にランダム生成された迷路を探索し、ゴールを目指すトップダウン視点の3Dシューティング迷路ゲームです。

- **移動:** WASD でプレイヤーを移動
- **射撃:** 左クリックでマウスカーソル方向に弾丸を発射（プレイヤーの移動慣性が弾に乗る）
- **チェックポイント:** 迷路内の行き止まりに配置されたチェックポイントブロックを回収（スコアに影響）
- **ゴール:** ゴールブロックに到達するとゲームクリア
- **敵:** プレイヤーから離れた床セルに出現し、**迷路の通路に沿って最短経路で追跡**。接触で HP が1減少
- **HP & TIME:** HP の初期値は5。制限時間は5分。HP が0になるか制限時間を超過するとゲームオーバー
- **ランク:** クリア時の残り時間とチェックポイント取得率の平均で S/A/B/C/D を算出

## 🧠 アルゴリズム概要 (Algorithm Overview)

| 機能 | アルゴリズム / データ構造 | 計算量（迷路 N×N、敵 E 体） | 狙い |
|---|---|---|---|
| 迷路生成 | 穴掘り法（スタックによる反復 DFS） | O(N²) | 毎回異なる完全迷路を生成、再帰によるスタックオーバーフローを回避 |
| ゴール・CP 配置 | BFS（最遠セル探索・行き止まり検出） | O(N²) | どんな迷路でもゴールが最遠地点になり難易度を保証 |
| 敵の経路決定 | プレイヤー起点 BFS による距離場（フローフィールド） | O(N²) / フレーム（E に依存しない） | 全敵が 1 回の BFS 結果を共有し、敵が増えても探索コストが増えない |
| 敵のスポーン位置 | 円周上の角度走査 + グリッドスナップ | O(360/Δθ) | 壁の中に出現させず、必ずプレイヤーから一定距離に出現させる |
| 弾・敵の管理 | オブジェクトプール（固定長 `vector` + `active` フラグ） | 取得 O(プールサイズ) | ゲーム中の `new`/`delete` をゼロにしメモリ断片化を防止 |
| 壁との衝突 | 軸分離 AABB（4 隅判定） | O(1) | 壁沿いのスライド移動・角への引っ掛かり防止 |
| 難易度曲線 | 双曲線関数によるスポーン間隔の短縮 | O(1) | ゼロ割り・間隔 0 を起こさず滑らかに難易度を上昇 |

## 🚀 技術的なこだわり (Technical Highlights)

### 1. 迷路自動生成（穴掘り法 / 反復 DFS）

全セルを壁で初期化し、スタックを使った深さ優先探索で「2 マス先の未訪問セル」へ壁を掘り進める穴掘り法（Recursive Backtracker）で、ゲーム開始ごとにランダムな完全迷路を生成しています。

```cpp
// Dungeon.cpp — スタックを使った反復実装
std::vector<int> order = {0, 1, 2, 3, 0, 1, 2, 3}; // モジュロ演算を省くための2周配列

while (!stk.empty())
{
    auto [nowI, nowJ] = stk.top();
    int startIndex = rand() % 4;          // ランダムな方向から探索開始
    bool all = true;
    for (int k = 0; k < 4; k++)
    {
        int newI = nowI + di[order[startIndex + k]] * 2;   // 2マス先
        int newJ = nowJ + dj[order[startIndex + k]] * 2;
        if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize)) continue;
        if (m_grid[newI][newJ] == CellType::WALL)          // 未訪問なら
        {
            m_grid[nowI + di[...]][nowJ + dj[...]] = CellType::FLOOR; // 間の壁を掘る
            m_grid[newI][newJ] = CellType::FLOOR;
            stk.push({newI, newJ});
            all = false; break;
        }
    }
    if (all) stk.pop();                   // 行き止まりならバックトラック
}
```

- **再帰ではなく明示的スタック**を使うことで、迷路サイズを大きくしてもコールスタックがあふれない
- 奇数グリッド（31×31）の奇数座標だけを通路の節点とすることで、壁厚 1 マス・ループのない**完全迷路**（任意の 2 点間の経路が一意）を保証
- `{0,1,2,3,0,1,2,3}` の 2 周配列でランダムオフセット付きの方向列挙を剰余演算なしで実現

### 2. BFS によるゴール・チェックポイント配置

迷路生成直後に、スタートを始点とした BFS を 1 回だけ実行し、「最遠セルの探索」と「行き止まりの検出」を同時に行っています。

```cpp
// Dungeon.cpp — 1 回の BFS で最遠セルと行き止まりを同時に取得
while (!que.empty())
{
    auto [nowI, nowJ, nowDist] = que.front(); que.pop();
    if (distL1[nowI][nowJ] != -1) continue;       // 訪問済み
    distL1[nowI][nowJ] = nowDist;
    if (maxDist < nowDist) { maxDist = nowDist; goalGrid = {nowI, nowJ}; }

    int freeDirCount = 0;
    for (int k = 0; k < 4; k++)
        if (/* 隣接セルが FLOOR */) { freeDirCount++; que.push({newI, newJ, nowDist + 1}); }

    if (freeDirCount == 1) deadEnds.push_back({nowI, nowJ}); // 通路が1方向 = 行き止まり
}
m_grid[goalGrid.first][goalGrid.second] = CellType::GOAL;
```

- 完全迷路では BFS 距離がそのまま「実際に歩く距離」になるため、**ゴールが常にスタートから最も遠い通路上**に置かれ、どんな迷路でも一定の探索量を保証
- 行き止まりをチェックポイント候補にすることで、「寄り道するほどスコアが上がる」リスク/リターンを自動で生成
- BFS 訪問順の先頭 2 要素（スタート直近）の行き止まりを除外し、開始直後に取れてしまうチェックポイントを防止
- 候補数が足りない迷路でも `std::min` でチェックポイント数をクランプし、生成が破綻しない

### 3. 敵の経路決定（BFS 距離場 / フローフィールド）【v2 で追加】

v1.x では敵がプレイヤーへの直線ベクトルで移動し、壁をすり抜けて迫ってくる仕様でしたが、v2 では**プレイヤーを始点とした BFS で全床セルの「プレイヤーまでの歩行距離」を毎フレーム計算**し、各敵はその距離場を下る方向へ移動するようにしました。

```cpp
// Player.cpp — プレイヤー起点の BFS で距離場（コストグリッド）を構築
void Player::CalcCostGrid(const Dungeon &dungeon)
{
    m_costGrid.assign(gridSize, std::vector<int>(gridSize, -1)); // -1 = 到達不可能
    que.push({0, {playerGridZ, playerGridX}});
    while (!que.empty())
    {
        auto [cost, pos] = que.front(); que.pop();
        if (m_costGrid[pos.first][pos.second] != -1) continue;
        m_costGrid[pos.first][pos.second] = cost;
        for (int i = 0; i < 4; i++)
        {
            // 範囲外・壁を除いた4近傍を cost + 1 でキューに積む
            if (dungeon.GetGridType(nx, nz) == Dungeon::CellType::WALL) continue;
            que.push({cost + 1, {nz, nx}});
        }
    }
}
```

```cpp
// EnemyManager.cpp — 4近傍のうち最もコストが低いセルの中心へ向かう
for (int i = 0; i < 4; i++)
{
    int cost = player.GetMoveCostToPlayer(nx, nz);
    if (cost == -1) continue;                    // 壁・到達不可能セル
    if (cost < minCost) { minCost = cost; ansX = nx; ansZ = nz; }
}
XMFLOAT3 fTo = dungeon.GridToWorld(ansX, ansZ);  // 次セルの中心への方向ベクトル
```

- **経路探索を「敵ごと」ではなく「プレイヤー 1 回」で済ませる設計**。敵 E 体それぞれが A* や BFS を行うと O(E·N²) だが、距離場を共有することで O(N²) + O(4E) に削減し、敵が最大 50 体に増えても探索コストは一定
- 各敵の判断は「4 近傍の値を見るだけ」の O(1) になり、敵が互いに独立して正しい最短経路を辿る
- 移動目標を**隣接セルの中心**にすることで、通路の角で壁に擦れず、グリッドに沿った自然な追跡になる
- プレイヤーと同じセルに入った後はワールド座標の直接ベクトルに切り替え、セル内での取り逃がしを防止
- 到達不可能セル（`-1`）や移動先がない場合は停止させ、未定義の方向に進まないよう安全側に倒している

### 4. 敵のスポーン位置探索（角度走査 + グリッドスナップ）【v2 で刷新】

v1.x では迷路中心を原点とする外周の円上（迷路の外側）に出現させていましたが、敵が通路を辿るようになった v2 では通路上に出現させる必要があるため、**プレイヤーを中心とする半径 R の円周上を角度走査し、最初に見つかった床セルに出現させる**方式に変更しています。

```cpp
// EnemyManager.cpp — ランダムな角度から10度刻みで一周走査
int degree = rand() % 360;
int startDegree = degree;
while (!isEnemyPlaced)
{
    float radian = XM_PI * degree / 180.f;
    float norm = ENEMYMANAGER_SPAWN_RADIUS * dungeon.getCellSize();
    pos.x = norm * std::cos(radian) + player.GetPosition().x;
    pos.z = norm * std::sin(radian) + player.GetPosition().z;

    if (dungeon.IsFloor(pos)) isEnemyPlaced = true;
    else { degree += ENEMYMANAGER_DELTA_DEGREE; degree %= 360; }

    if (degree == startDegree) break;   // 一周しても床がなければ諦める（無限ループ防止）
}
// WorldToGrid → GridToWorld で床セルの中心にスナップ
```

- 開始角度をランダムにしつつ、失敗時は決定的に走査するため、**必ず有限回で終了**する
- 出現距離を固定半径にすることで、プレイヤーの目の前に突然湧く理不尽さを排除
- 出現座標をセル中心にスナップし、手順 3 の経路追跡（隣接セル中心を目指す）と整合させている

### 5. オブジェクトプールパターン（弾丸・敵）

弾丸と敵を毎フレーム `new`/`delete` せず、`Init()` 時に固定サイズで確保したプールを `active` フラグで使い回しています。

```cpp
// ProjectilePool.cpp — 非アクティブなスロットを返す（満杯なら nullptr）
Projectile *ProjectilePool::Get()
{
    for (size_t i = 0; i < m_pool.size(); i++)
        if (!m_pool[i].active) return &m_pool[i];
    return nullptr; // 上限を超えて生成しない
}
```

- ゲームループ中の動的メモリ確保をゼロにし、アロケーションコストとメモリ断片化を排除
- 要素を連続メモリ（`std::vector`）に配置するため、更新・描画ループがキャッシュフレンドリー
- 弾丸プール（50 発）・敵プール（50 体）を分離し、上限到達時は生成をスキップすることで**最悪ケースの負荷に上限**を設けている
- 敵・弾ともに「データのみの struct」+「ロジックを持つ Manager/Pool」に分離し、データ指向に近い構成

### 6. AABB 衝突判定と軸分離移動（壁スライド）

プレイヤーの X 軸と Z 軸の移動を独立に解決することで、壁に対するスライド移動を実現しています。

```cpp
// Player.cpp — X→Z の順に独立した壁衝突解決
float newX = m_position.x + m_velVec.x * m_speed * deltaTime;
if (dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) || ...) newX = m_position.x;

float newZ = m_position.z + m_velVec.z * m_speed * deltaTime;
if (dungeon.IsWall(newX - HALF_SIZE, newZ - HALF_SIZE) || ...) newZ = m_position.z;
```

```cpp
// Collision.cpp — AABB（2D）衝突判定
bool Collision::checkAABB(const BBOX &box1, const BBOX &box2)
{
    return box1.minX <= box2.maxX && box1.maxX >= box2.minX &&
           box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ;
}
```

- 壁判定はワールド座標 → グリッド座標変換による O(1) のセル参照で行い、壁の数に依存しない
- バウンディングボックスの 4 隅すべてをチェックして角のめり込みを防止
- Z 判定には確定後の `newX` を使うことで対角コーナーへの引っ掛かりを解消

### 7. 難易度曲線と射撃の物理

```cpp
// EnemyManager.cpp — 経過時間に応じてスポーン間隔を双曲線的に短縮
m_spawnInterval = ENEMYMANAGER_SPAWN_INTERVAL / (1.0f + m_elapsedTime / ENEMYMANAGER_SPAWN_TIME_SCALE);
```

```cpp
// ProjectilePool.cpp — 弾の速度 = 照準方向 × 弾速 + プレイヤー速度（慣性）【v2 で追加】
dir_float3_norm.x = dir.x * proj->speed + playerVel.x * playerSpeed;
dir_float3_norm.z = dir.z * proj->speed + playerVel.z * playerSpeed;
```

- スポーン間隔は `t=0s: 3.0s → t=60s: 1.5s → t=300s: 0.5s` と滑らかに短縮。分母が常に 1 以上なのでゼロ割りも間隔 0 も発生しない
- 弾にプレイヤーの速度ベクトルを合成し、走りながら撃ったときの弾道を物理的に自然にした
- 照準はマウスのスクリーン座標をニア/ファー面へ `XMVector3Unproject` で逆変換し、得られたレイと水平面のパラメトリック交差（`t = (planeY - origin.y) / dir.y`）で算出

### 8. シーン・状態管理とサウンド【v2 で拡張】

- `GameState` を `HowToPlay → Prepare → Playing → GameClear / GameOver` に細分化し、操作説明・準備時間・リザルト演出を状態遷移として分離
- リザルト画面ではタイマー駆動でテキストを段階表示し、ランクが D→S へ 1 段ずつ上がる演出と SE を同期
- `SoundManager` をシングルトンで実装し、XAudio2 + dr_mp3 で BGM（ループ再生）/ SE を再生。SE は**起動時に一度だけデコードしてソースボイスを再利用**し、再生時のデコード・生成コストを排除（プールと同じ発想）
- コピーコンストラクタ・代入演算子を `= delete` にしてシングルトンの複製を禁止

### その他の実装

- **レンダリング:** D3D11 のデバイス・スワップチェイン・深度バッファを手動構築し、HLSL のブリン・フォンシェーディング（アンビエント・ディフューズ・スペキュラ・エミッシブ）で描画
- **カメラ:** プレイヤー頭上の固定オフセットから `XMMatrixLookAtLH` で追従するトップダウンカメラ
- **UI:** SpriteBatch による HUD、ホバー/押下の 3 状態で色が変わるボタン、ランクごとのレアリティ配色

## 🆕 v1.x からの変更点 (Changes since v1.x)

| 分類 | 変更内容 |
|---|---|
| **敵 AI** | 壁をすり抜ける直線追尾 → プレイヤー起点の BFS 距離場で通路に沿って追跡 |
| **敵スポーン** | 迷路外周からの出現 → プレイヤー周囲の円周上の床セルに出現。無限ループ・座標ずれのバグを修正 |
| **射撃** | 弾の速度にプレイヤーの移動慣性を合成 |
| **サウンド** | XAudio2 + dr_mp3 によるサウンドシステムを追加（タイトル/ゲーム/クリア/ゲームオーバー BGM、被弾・撃破・CP 取得・ボタン・ランクアップ SE、SE 音量の定数管理） |
| **ゲームフロー** | 操作説明 → 準備時間 → ゲーム開始の段階的な開始、シーン遷移直後の誤入力防止 |
| **リザルト** | フェードイン、テキストの段階表示、ランクアップ演出、ランクごとの配色 |
| **UI** | ボタンのホバー/押下状態の色変化 |
| **調整・リファクタ** | ゲーム難易度の調整、マジックナンバー・UI 定数の定数化、ランク管理を文字列から整数へ、メンバ変数の明示的初期化、インクルード整理 |

## 📂 ディレクトリ構成 (Project Structure)

```
MazeSurge/
├── include/MazeSurge/
│   ├── Audio/
│   │   └── SoundManager.h    # XAudio2 による BGM/SE 管理（シングルトン）
│   ├── Core/
│   │   ├── Common.h          # 共有インクルード・型定義
│   │   ├── Types.h           # Vertex, ConstantBuffer, BBOX 構造体
│   │   ├── Core.h            # コアヘッダのまとめ
│   │   └── GameConstant.h    # ゲーム全体の定数定義
│   ├── Game/
│   │   ├── Dungeon.h         # 迷路グリッド・生成・描画
│   │   ├── Player.h          # プレイヤー移動・HP・BFS 距離場
│   │   ├── Enemy.h           # 敵データ構造体
│   │   ├── EnemyManager.h    # 敵のスポーン・経路追跡・プール管理
│   │   ├── Projectile.h      # 弾丸データ構造体
│   │   └── ProjectilePool.h  # 弾丸プール・射撃処理
│   ├── Graphics/
│   │   ├── Renderer.h        # D3D11 リソース管理・描画・UI
│   │   ├── Camera.h          # 追従カメラ・スクリーン→ワールド変換
│   │   └── Cube.h            # キューブ頂点・インデックスデータ
│   ├── Scene/
│   │   ├── Scene.h           # シーン基底クラス・GameState
│   │   ├── TitleScene.h      # タイトルシーン
│   │   └── GameScene.h       # ゲームシーン（ゲームループ・状態管理）
│   ├── UI/
│   │   └── UIConstant.h      # UI レイアウト・演出タイミング定数
│   ├── Utility/
│   │   └── Collision.h       # AABB 衝突判定
│   └── External/
│       ├── dr_mp3.h          # MP3 デコーダ
│       └── tiny_obj_loader.h
├── src/
│   ├── main.cpp              # WinMain, WndProc, シーン切り替え, BGM 切り替え
│   ├── Audio/
│   │   ├── SoundManager.cpp  # MP3 デコード・BGM ループ再生・SE 再利用
│   │   └── dr_mp3.cpp        # dr_mp3 の実装部
│   ├── Game/
│   │   ├── Dungeon.cpp       # 穴掘り法・BFS・座標変換
│   │   ├── Player.cpp        # WASD 移動・壁衝突解決・BFS 距離場の構築
│   │   ├── Enemy.cpp         # 敵の初期化
│   │   ├── EnemyManager.cpp  # 角度走査スポーン・距離場による追跡・難易度曲線
│   │   ├── Projectile.cpp    # 弾丸の初期化
│   │   └── ProjectilePool.cpp# 射撃（慣性付き）・弾丸更新・衝突
│   ├── Graphics/
│   │   ├── Renderer.cpp      # パイプライン初期化・描画・SpriteBatch UI
│   │   └── Camera.cpp        # ビュー/プロジェクション行列・レイキャスト
│   ├── Scene/
│   │   ├── TitleScene.cpp    # タイトル描画・入力待ち
│   │   └── GameScene.cpp     # ゲームロジック・リザルト演出・ランク算出
│   └── Utility/
│       └── Collision.cpp     # AABB 判定実装
├── shaders/
│   └── shaders.hlsl          # 頂点/ピクセルシェーダー（ブリン・フォン）
└── Sounds/                   # BGM / SE（mp3）
```

## 📦 ビルド方法 (Build Instructions)

### 前提条件

- Windows 10 以降
- Visual Studio 2026（C++ デスクトップ開発ワークロード）

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

- **迷路自動生成:** 穴掘り法（スタックによる反復 DFS）による 31×31 の完全迷路生成
- **ゴール・CP 配置:** 1 回の BFS で最遠セルをゴールに、行き止まりをチェックポイントに配置
- **敵の経路追跡:** プレイヤー起点の BFS 距離場を全敵で共有し、4 近傍の最小コストセルへ移動
- **敵スポーン:** プレイヤー中心の円周を角度走査し、床セル中心にスナップして出現
- **オブジェクトプール:** 弾丸 50 発・敵 50 体を動的確保なしで使い回し
- **難易度曲線:** 経過時間に応じて双曲線的に短縮するスポーン間隔
- **WASD 移動:** X/Z 独立衝突解決による壁スライド移動
- **AABB 衝突判定:** プレイヤー・敵・弾丸・壁の 2D 矩形衝突判定
- **射撃システム:** スクリーン→ワールド逆変換による照準、プレイヤー慣性の合成
- **サウンド:** XAudio2 + dr_mp3 による BGM ループ再生・SE 再利用
- **シーン管理:** Title / HowToPlay / Prepare / Playing / GameClear / GameOver の状態遷移
- **スコア/ランク:** 残り時間比率とチェックポイント取得率の平均で S/A/B/C/D を算出し、段階演出で表示
