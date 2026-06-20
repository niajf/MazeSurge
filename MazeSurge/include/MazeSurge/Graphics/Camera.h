#pragma once
#include "MazeSurge/Core/Common.h"

// ============================================================
// Camera — フリーカメラの状態管理
// ============================================================
class Camera
{
public:
    // offsetY: カメラの高さ（上方向）
    // offsetZ: カメラの奥行きオフセット（後退方向）
    void Init(float offsetY = 14.0f, float offsetZ = -7.0f);

    // プレイヤーの位置を受け取りカメラ位置を更新する
    // main.cpp のゲームループから毎フレーム呼ぶ
    void Update(const XMFLOAT3 &targetPos);

    XMMATRIX GetViewMatrix() const;
    XMMATRIX GetProjectionMatrix() const;
    XMFLOAT3 GetPosition() const { return m_position; }

    // マウスのスクリーン座標をワールド空間の座標に変換する
    // 射撃の向き計算に使用する（Week3で使う）
    // planeY: 交差を計算する水平面のY座標（プレイヤーのY）
    XMFLOAT3 ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const;

private:
    XMFLOAT3 m_position = {0, 0, 0}; // カメラの実際の位置
    XMFLOAT3 m_target = {0, 0, 0};   // 注視点（プレイヤーの位置）
    float m_offsetY = 14.0f;         // プレイヤーからの垂直オフセット
    float m_offsetZ = -7.0f;         // プレイヤーからの奥行きオフセット
};