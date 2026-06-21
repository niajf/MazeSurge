#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <algorithm>

// ============================================================
// Player — プレイヤーの状態管理・移動・描画
// ============================================================
// 移動はキー入力に基づく等速直線移動で、X 軸と Z 軸を独立して解決する
// 軸分離 AABB スライド方式により壁沿い移動に対応している。
class Player
{
public:
    // 初期位置を受け取り、色・スケール・速度・HP・BBOX を初期化する。
    void Init(const XMFLOAT3 &staticPosition);
    // 毎フレーム呼ぶ。入力取得・移動計算・壁衝突解決・BBOX 更新を行う。
    void Update(float deltaTime, Dungeon &g_dungeon, const InputState &inputState);
    // プレイヤーキューブをレンダラーに描画させる。
    void Draw(Renderer &renderer) const;
    // 敵との衝突時に呼ばれ、HP を 1 減らす。
    // GameOver 判定は GameScene::Update() が HP <= 0 を検知して行う。
    void hitEnemy();

    XMFLOAT3 GetPosition() const { return m_position; }
    BBOX GetBBOX() const { return m_bbox; }
    int GetHP() const { return m_hp; }

private:
    // WASD の押下状態を読み取り、斜め移動を正規化した速度ベクトルを返す。
    // private にしているのは Update() 内部の計算ステップであり、外部から直接呼ぶ必要がないため。
    XMFLOAT3 CalcMoveVelocity() const;

    // InputState から毎フレームコピーするキー状態。
    // CalcMoveVelocity() が const メンバのため、入力状態をメンバに持つ必要がある。
    bool m_keyW = false; // W キー（前進 +Z）
    bool m_keyA = false; // A キー（左移動 -X）
    bool m_keyS = false; // S キー（後退 -Z）
    bool m_keyD = false; // D キー（右移動 +X）

    XMFLOAT4 m_color;    // プレイヤーキューブの描画色
    XMFLOAT3 m_position; // ワールド座標（Y=0 固定、床面上）
    BBOX m_bbox;         // 衝突判定用 AABB（XZ 平面のみ）
    float m_scale;       // キューブの辺長（均一スケール）
    float m_speed;       // 移動速度（単位/秒）
    int m_hp;            // 残り HP
};