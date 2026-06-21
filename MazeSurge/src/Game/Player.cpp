#include "MazeSurge/Game/Player.h"

void Player::Init(const XMFLOAT3 &startPosition)
{
	m_color = PLAYER_CELL_COLOR;
	m_position = startPosition;
	m_bbox.setBBOX(m_position, m_scale);
	m_scale = PLAYER_CELL_SCALE;
	m_speed = PLAYER_MOVE_SPEED;
	m_hp = PLAYER_HP;
}

XMFLOAT3 Player::CalcMoveVelocity() const
{
	XMFLOAT3 velocity = {0.0f, 0.0f, 0.0f};

	if (m_keyW)
		velocity.z += 1.0f;
	if (m_keyS)
		velocity.z -= 1.0f;
	if (m_keyD)
		velocity.x += 1.0f;
	if (m_keyA)
		velocity.x -= 1.0f;

	XMVECTOR v = XMLoadFloat3(&velocity);
	if (XMVectorGetX(XMVector3Length(v)) > 0.0f)
		v = XMVector3Normalize(v);

	XMStoreFloat3(&velocity, v);
	return velocity;
}

void Player::Update(float deltaTime, Dungeon &g_dungeon, const InputState &inputState)
{
	constexpr float HALF_SIZE = PLAYER_CELL_SCALE * 0.5f;

	m_keyW = inputState.keyW;
	m_keyS = inputState.keyS;
	m_keyD = inputState.keyD;
	m_keyA = inputState.keyA;

	XMFLOAT3 velocity = CalcMoveVelocity();

	// X軸の移動と壁衝突判定（4コーナーをチェック）
	float newX = m_position.x + velocity.x * m_speed * deltaTime;
	if (g_dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, m_position.z - HALF_SIZE) ||
		g_dungeon.IsWall(newX - HALF_SIZE, m_position.z + HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, m_position.z + HALF_SIZE))
	{
		newX = m_position.x;
	}

	// Z軸の移動と壁衝突判定（新しいXを使って4コーナーをチェック）
	float newZ = m_position.z + velocity.z * m_speed * deltaTime;
	if (g_dungeon.IsWall(newX - HALF_SIZE, newZ - HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, newZ - HALF_SIZE) ||
		g_dungeon.IsWall(newX - HALF_SIZE, newZ + HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, newZ + HALF_SIZE))
	{
		newZ = m_position.z;
	}

	m_position.x = newX;
	m_position.z = newZ;

	m_bbox.setBBOX(m_position, m_scale);
}

void Player::Draw(Renderer &renderer) const
{
	XMMATRIX world = XMMatrixScaling(m_scale, m_scale, m_scale) * XMMatrixTranslation(m_position.x, m_position.y + PLAYER_CELL_SCALE * 0.5f, m_position.z);
	renderer.DrawCube(world, m_color);
}

void Player::hitEnemy()
{
	m_hp--;
}