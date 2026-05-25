#include "MazeSurge/Player.h"
#include "MazeSurge/Renderer.h"
#include <algorithm>

Player g_player;

void Player::Init(const XMFLOAT3& startPosition)
{
	m_position = startPosition;
	m_hp = 100;
}

XMFLOAT3 Player::CalcMoveVelocity() const
{
	XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f };

	if (keyW) velocity.z += 1.0f;
	if (keyS) velocity.z -= 1.0f;
	if (keyD) velocity.x += 1.0f;
	if (keyA) velocity.x -= 1.0f;

	XMVECTOR v = XMLoadFloat3(&velocity);
	if (XMVectorGetX(XMVector3Length(v)) > 0.0f)
		v = XMVector3Normalize(v);

	XMStoreFloat3(&velocity, v);
	return velocity;
}

void Player::Update(float deltaTime)
{
	XMFLOAT3 velocity = CalcMoveVelocity();

	// 仮の移動先を計算
	XMFLOAT3 newPos;
	newPos.x = m_position.x + velocity.x * m_speed * deltaTime;
	newPos.y = m_position.y;
	newPos.z = m_position.z + velocity.z * m_speed * deltaTime;

	wchar_t buf[128];
	swprintf_s(buf, L"Player pos: %.1f, %.1f, %.1f\n",
		m_position.x, m_position.y, m_position.z);
	OutputDebugString(buf);

	// 仮の移動制限
	constexpr float BOUNDARY = 20.0f;
	newPos.x = std::clamp(newPos.x, -BOUNDARY, BOUNDARY);
	newPos.z = std::clamp(newPos.z, -BOUNDARY, BOUNDARY);

	m_position = newPos;
}

void Player::Draw() const
{
	XMMATRIX world = XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(m_position.x, m_position.y + 0.4f, m_position.z);
	XMFLOAT4 playerColor = { 0.0f, 1.0f, 1.0f, 1.0f };
	g_renderer.DrawCube(world, playerColor);
}