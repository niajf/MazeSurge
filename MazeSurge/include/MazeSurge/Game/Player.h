#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <algorithm>

class Player
{
public:
    void Init(const XMFLOAT3 &staticPosition);                                      // 初期位置を受け取り各パラメータを初期化する
    void Update(float deltaTime, Dungeon &g_dungeon, const InputState &inputState); // 入力・移動・衝突判定を毎フレーム処理する
    void Draw(Renderer &renderer) const;                                            // プレイヤーキューブを描画する
    void hitEnemy();                                                                // 敵に当たったときに HP を1減らす

    XMFLOAT3 GetPosition() const { return m_position; }; // プレイヤーの現在座標を返す
    BBOX GetBBOX() const { return m_bbox; };             // プレイヤーの当たり判定領域を返す
    int GetHP() const { return m_hp; };                  // 現在の HP 値を返す

private:
    // WASD の押下状態から正規化された移動ベクトルを返す
    XMFLOAT3 CalcMoveVelocity() const;

    // キーの入力状況を保持する変数
    bool m_keyW = false; // W キー（前進）
    bool m_keyA = false; // A キー（左移動）
    bool m_keyS = false; // S キー（後退）
    bool m_keyD = false; // D キー（右移動）

    XMFLOAT4 m_color;    // プレイヤーブロックの色
    XMFLOAT3 m_position; // プレイヤーの座標
    BBOX m_bbox;         // プレイヤーのBoundingBox
    float m_scale;       // プレイヤーブロックのスケール
    float m_speed;       // プレイヤーの移動速度
    int m_hp;            // プレイヤーのHP
};