#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Camera — フリーカメラの状態管理
// ============================================================
class Camera
{
public:
    void Init();

    // プレイヤーの位置を受け取りカメラ位置を更新する
    // main.cpp のゲームループから毎フレーム呼ぶ
    void Update(const XMFLOAT3 &targetPos);

    XMMATRIX GetViewMatrix() const;
    XMMATRIX GetProjectionMatrix() const;
    XMFLOAT3 GetPosition() const { return m_position; }

    // マウスのスクリーン座標をワールド空間の座標に変換する
    // planeY: 交差を計算する水平面のY座標（プレイヤーのY）
    XMFLOAT3 ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const;

private:
    XMFLOAT3 m_position; // カメラの実際の位置
    XMFLOAT3 m_target;   // 注視点（プレイヤーの位置）
    float m_offsetY;     // プレイヤーからの垂直オフセット
    float m_offsetZ;     // プレイヤーからの奥行きオフセット
    float m_fovDegree;   // 垂直視野角（度数）
};