#include "MazeSurge/Player.h"


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

void Player::Update(float deltaTime, const Dungeon& dungeon)
{
	constexpr float HALF_SIZE = 0.4f;

	XMFLOAT3 velocity = CalcMoveVelocity();

	// X軸の移動と壁衝突判定（4コーナーをチェック）
	float newX = m_position.x + velocity.x * m_speed * deltaTime;
	if (dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, m_position.z - HALF_SIZE) ||
		dungeon.IsWall(newX - HALF_SIZE, m_position.z + HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, m_position.z + HALF_SIZE))
	{
		newX = m_position.x;
	}

	// Z軸の移動と壁衝突判定（新しいXを使って4コーナーをチェック）
	float newZ = m_position.z + velocity.z * m_speed * deltaTime;
	if (dungeon.IsWall(newX - HALF_SIZE, newZ - HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, newZ - HALF_SIZE) ||
		dungeon.IsWall(newX - HALF_SIZE, newZ + HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, newZ + HALF_SIZE))
	{
		newZ = m_position.z;
	}

	m_position.x = newX;
	m_position.z = newZ;
}

void Player::Draw() const
{
	XMMATRIX world = XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(m_position.x, m_position.y + 0.4f, m_position.z);
	XMFLOAT4 playerColor = { 0.0f, 1.0f, 1.0f, 1.0f };
	g_renderer.DrawCube(world, playerColor);
}