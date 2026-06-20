#include "MazeSurge/Graphics/Camera.h"

void Camera::Init()
{
    m_offsetY = CAMERA_OFFSET_Y;
    m_offsetZ = CAMERA_OFFSET_Z;
    m_fovDegree = CAMERA_FOV_DEG;
}

void Camera::Update(const XMFLOAT3 &targetPos)
{
    // プレイヤーの位置を保持
    m_target = targetPos;

    // カメラ位置はプレイヤー位置 + オフセット
    m_position.x = targetPos.x;
    m_position.y = targetPos.y + m_offsetY;
    m_position.z = targetPos.z + m_offsetZ;
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR eye = XMLoadFloat3(&m_position);
    XMVECTOR target = XMLoadFloat3(&m_target);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    return XMMatrixLookAtLH(eye, target, up);
}

XMMATRIX Camera::GetProjectionMatrix() const
{
    float fov = XMConvertToRadians(m_fovDegree);
    float aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    return XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 100.0f);
}

XMFLOAT3 Camera::ScreenToWorldOnPlane(int mouseX, int mouseY, float planeY) const
{
    // ニアクリップ面とファークリップ面上の点をワールド空間に逆変換し、
    // そのレイとY=planeYの水平面との交点を返す
    XMVECTOR nearScreen = XMVectorSet(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        0.0f, 0.0f);
    XMVECTOR farScreen = XMVectorSet(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        1.0f, 0.0f);

    XMMATRIX view = GetViewMatrix();
    XMMATRIX proj = GetProjectionMatrix();
    XMMATRIX world = XMMatrixIdentity();

    XMVECTOR nearWorld = XMVector3Unproject(
        nearScreen,
        0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
        0.0f, 1.0f, proj, view, world);

    XMVECTOR farWorld = XMVector3Unproject(
        farScreen,
        0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
        0.0f, 1.0f, proj, view, world);

    // レイの方向
    XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(farWorld, nearWorld));

    // Y=planeY の平面との交点を計算
    // nearWorld.y + t * dir.y = planeY
    float originY = XMVectorGetY(nearWorld);
    float dirY = XMVectorGetY(dir);

    // dirYがほぼ0 = レイが水平（交点なし）
    if (fabsf(dirY) < 0.0001f)
        return {0.0f, planeY, 0.0f};

    float t = (planeY - originY) / dirY;

    // 交点 = origin + t * direction
    XMVECTOR hitPos = XMVectorAdd(nearWorld, XMVectorScale(dir, t));

    XMFLOAT3 result;
    XMStoreFloat3(&result, hitPos);
    return result;
}