#include "MazeSurge/Game/Player.h"

void Player::Init(const XMFLOAT3 &startPosition)
{
	m_color = PLAYER_CELL_COLOR;
	m_position = startPosition;
	m_scale = PLAYER_CELL_SCALE;
	m_speed = PLAYER_MOVE_SPEED;
	m_hp = PLAYER_HP;
	// bbox は position と scale が確定してから設定する。
	m_bbox.setBBOX(m_position, m_scale);
}

XMFLOAT3 Player::CalcMoveVelocity() const
{
	XMFLOAT3 velocity = {0.0f, 0.0f, 0.0f};

	// WASD それぞれ独立して速度に加算する。
	// 斜め移動時は正規化前のベクトル長が √2 になるため、後段で正規化する。
	if (m_keyW)
		velocity.z += 1.0f;
	if (m_keyS)
		velocity.z -= 1.0f;
	if (m_keyD)
		velocity.x += 1.0f;
	if (m_keyA)
		velocity.x -= 1.0f;

	XMVECTOR v = XMLoadFloat3(&velocity);
	// 斜め移動でも縦横移動と同じ速度になるよう正規化する。
	// 長さ 0（無入力）の場合は正規化をスキップしてゼロベクトルのままにする。
	if (XMVectorGetX(XMVector3Length(v)) > 0.0f)
		v = XMVector3Normalize(v);

	XMStoreFloat3(&velocity, v);
	return velocity;
}

void Player::Update(float deltaTime, Dungeon &g_dungeon, const InputState &inputState)
{
	// HALF_SIZE: バウンディングボックスの半辺長。壁判定の 4 隅オフセットに使う。
	// constexpr にすることで毎フレームの乗算コストをゼロにする。
	constexpr float HALF_SIZE = PLAYER_CELL_SCALE * 0.5f;

	// WndProc で更新された InputState をメンバにコピーして CalcMoveVelocity に渡す。
	m_keyW = inputState.keyW;
	m_keyS = inputState.keyS;
	m_keyD = inputState.keyD;
	m_keyA = inputState.keyA;

	XMFLOAT3 velocity = CalcMoveVelocity();

	// ---- X 軸移動と壁衝突判定 ----
	// バウンディングボックスの 4 隅すべてで IsWall を判定することで、
	// キューブの角が壁にめり込むケース（1 点だけ壁内に入る）を防ぐ。
	float newX = m_position.x + velocity.x * m_speed * deltaTime;
	if (g_dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, m_position.z - HALF_SIZE) ||
		g_dungeon.IsWall(newX - HALF_SIZE, m_position.z + HALF_SIZE) ||
		g_dungeon.IsWall(newX + HALF_SIZE, m_position.z + HALF_SIZE))
	{
		// 壁に当たった場合は X 位置を変えない（壁へのめり込みを防ぐ）。
		newX = m_position.x;
	}

	// ---- Z 軸移動と壁衝突判定 ----
	// Z 判定には X で確定した newX を使う。
	// これにより対角コーナーへの引っ掛かりを防ぎ、壁沿いのスライド移動が可能になる。
	// X→Z の順に独立して解決するのが「軸分離 AABB スライド」の基本パターン。
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

	// 位置が変わったので bbox を再計算する。
	m_bbox.setBBOX(m_position, m_scale);
}

void Player::Draw(Renderer &renderer) const
{
	// キューブのピボットはモデル原点（底面中心）に合わせるため、
	// Translation の Y に PLAYER_CELL_SCALE * 0.5f を加算して床面から浮かせる。
	XMMATRIX world = XMMatrixScaling(m_scale, m_scale, m_scale) * XMMatrixTranslation(m_position.x, m_position.y + PLAYER_CELL_SCALE * 0.5f, m_position.z);
	renderer.DrawCube(world, m_color);
}

void Player::hitEnemy()
{
	// 敵との衝突 1 回につき HP を 1 減らす。
	// 0 以下になった場合の GameOver 判定は GameScene::Update() 側で行う。
	m_hp--;
}
