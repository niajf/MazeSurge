#include "MazeSurge/Graphics/Camera.h"

void Camera::Init()
{
    m_offsetY = CAMERA_OFFSET_Y; // プレイヤー頭上への垂直オフセット
    m_offsetZ = CAMERA_OFFSET_Z; // プレイヤー後方への奥行きオフセット
    m_fovDegree = CAMERA_FOV_DEG;
}

void Camera::Update(const XMFLOAT3 &targetPos)
{
    // 注視点をプレイヤー位置に固定する。
    m_target = targetPos;

    // カメラ位置 = プレイヤー位置 + オフセット（見下ろし俯瞰視点）。
    // X は真上から追いかける（横ブレなし）。
    m_position.x = targetPos.x;
    m_position.y = targetPos.y + m_offsetY;
    m_position.z = targetPos.z + m_offsetZ;
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR eye = XMLoadFloat3(&m_position);
    XMVECTOR target = XMLoadFloat3(&m_target);
    // up ベクトルは常にワールド Y 軸上方向。カメラロールなし。
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // LH = 左手座標系（DirectX の標準）。
    return XMMatrixLookAtLH(eye, target, up);
}

XMMATRIX Camera::GetProjectionMatrix() const
{
    float fov = XMConvertToRadians(m_fovDegree);
    // アスペクト比は実行時ウィンドウサイズから算出する。定数で固定しないことで
    // 解像度変更に対応できる（現在はウィンドウサイズ固定だが拡張余地を残す）。
    float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    // LH 透視投影。near/far クリップは定数で定義済み。
    return XMMatrixPerspectiveFovLH(fov, aspect, CAMERA_NEAR_CLIP, CAMERA_FAR_CLIP);
}

XMFLOAT3 Camera::ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const
{
    // スクリーン座標をニアクリップ面(z=0)とファークリップ面(z=1)の 2 点に変換し、
    // そのレイと Y=planeY の水平面との交点をワールド座標で返す。
    // 弾の照準計算に使用する（クリック位置 → 発射方向）。
    XMVECTOR nearScreen = XMVectorSet(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        0.0f, 0.0f); // z=0: ニアクリップ面

    XMVECTOR farScreen = XMVectorSet(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        1.0f, 0.0f); // z=1: ファークリップ面

    XMMATRIX view = GetViewMatrix();
    XMMATRIX proj = GetProjectionMatrix();
    // ワールド変換は単位行列（スクリーン→ワールドの逆変換なのでモデル行列は不要）。
    XMMATRIX world = XMMatrixIdentity();

    // XMVector3Unproject でスクリーン座標をワールド空間へ逆変換する。
    XMVECTOR nearWorld = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
        0.0f, 1.0f, proj, view, world);

    XMVECTOR farWorld = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
        0.0f, 1.0f, proj, view, world);

    // nearWorld → farWorld 方向へのレイベクトル（正規化）。
    XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(farWorld, nearWorld));

    // Y=planeY の水平面との交点を求める。
    // 直線の媒介変数表示: P(t) = nearWorld + t * dir
    // Y 成分が planeY になる t を解く: nearWorld.y + t * dir.y = planeY
    float originY = XMVectorGetY(nearWorld);
    float dirY = XMVectorGetY(dir);

    // dirY ≈ 0 はレイがほぼ水平（カメラが水平を向いている異常状態）。
    // この場合は交点が存在しないため、プレイヤー足元付近を返してフォールバックする。
    if (fabsf(dirY) < 0.0001f)
        return {0.0f, planeY, 0.0f};

    float t = (planeY - originY) / dirY;

    // 交点座標 = nearWorld + t * dir
    XMVECTOR hitPos = XMVectorAdd(nearWorld, XMVectorScale(dir, t));

    XMFLOAT3 result;
    XMStoreFloat3(&result, hitPos);
    return result;
}
