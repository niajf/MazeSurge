#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Camera — 三人称俯瞰カメラの状態管理
// ============================================================
// プレイヤーを常に注視点とし、真上やや後方から見下ろす固定オフセットカメラ。
// 行列計算には DirectXMath の左手座標系（LH）関数を使用する。
class Camera
{
public:
    // CAMERA_* 定数でオフセット・FOV を初期化する。
    void Init();

    // プレイヤーの最新位置を受け取り、カメラ位置と注視点を更新する。
    // ゲームループで Player::Update() の直後に呼ぶこと。
    void Update(const XMFLOAT3 &targetPos);

    // LH ビュー行列（eye・target・up から計算）を返す。
    XMMATRIX GetViewMatrix() const;
    // LH 透視投影行列（FOV・アスペクト比・near/far から計算）を返す。
    XMMATRIX GetProjectionMatrix() const;
    // カメラのワールド座標を返す（スペキュラー計算で LightBuffer に渡す）。
    XMFLOAT3 GetPosition() const { return m_position; }

    // スクリーン座標（マウス位置）をワールド空間の Y=planeY 水平面上の座標に変換する。
    // ニア/ファークリップ面への逆投影でレイを生成し、水平面との交点を返す。
    // planeY: 交差を求める水平面の Y 座標（弾の発射高さに合わせる）。
    XMFLOAT3 ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const;

private:
    XMFLOAT3 m_position; // カメラのワールド座標（Update() で毎フレーム計算）
    XMFLOAT3 m_target;   // 注視点（= プレイヤーの位置）
    float m_offsetY;     // プレイヤーから上方向へのオフセット（俯瞰の高さ）
    // m_offsetZ が負値のとき、カメラはプレイヤーより手前（-Z 方向）に位置する。
    // これにより斜め下方向への見下ろし視点が生まれる。
    float m_offsetZ;
    float m_fovDegree; // 垂直視野角（度）
};
