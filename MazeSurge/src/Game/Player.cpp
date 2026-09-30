#include "MazeSurge/Game/Player.h"

void Player::Init(const XMFLOAT3 &startPosition)
{
	m_color = PLAYER_CELL_COLOR;
	m_position = startPosition;
	m_velVec = XMFLOAT3{0.f, 0.f, 0.f};
	m_scale = PLAYER_CELL_SCALE;
	m_speed = PLAYER_MOVE_SPEED;
	m_hp = PLAYER_HP;

	// bbox は position と scale が確定してから設定する
	m_bbox.setBBOX(m_position, m_scale);
}

XMFLOAT3 Player::CalcMoveVelocity()
{
	XMFLOAT3 velocity = XMFLOAT3{0.f, 0.f, 0.f};

	// WASD それぞれ独立して速度に加算する
	// 斜め移動時は正規化前のベクトル長が √2 になるため、後段で正規化する
	if (m_keyW)
		velocity.z += 1.0f;
	if (m_keyS)
		velocity.z -= 1.0f;
	if (m_keyD)
		velocity.x += 1.0f;
	if (m_keyA)
		velocity.x -= 1.0f;

	XMVECTOR v = XMLoadFloat3(&velocity);

	// 斜め移動でも縦横移動と同じ速度になるよう正規化する
	// 長さ 0（無入力）の場合は正規化をスキップしてゼロベクトルのままにする
	if (XMVectorGetX(XMVector3Length(v)) > 0.0f)
		v = XMVector3Normalize(v);

	XMStoreFloat3(&velocity, v);
	return velocity;
}

void Player::Update(float deltaTime, Dungeon &dungeon, const InputState &inputState)
{
	// HALF_SIZE: バウンディングボックスの半辺長壁判定の 4 隅オフセットに使う
	// constexpr にすることで毎フレームの乗算コストをゼロにする
	constexpr float HALF_SIZE = PLAYER_CELL_SCALE * 0.5f;

	// WndProc で更新された InputState をメンバにコピーして CalcMoveVelocity に渡す
	m_keyW = inputState.keyW;
	m_keyS = inputState.keyS;
	m_keyD = inputState.keyD;
	m_keyA = inputState.keyA;

	m_velVec = CalcMoveVelocity();

	// ---- X 軸移動と壁衝突判定 ----
	// バウンディングボックスの 4 隅すべてで IsWall を判定することで、
	// キューブの角が壁にめり込むケース（1 点だけ壁内に入る）を防ぐ
	float newX = m_position.x + m_velVec.x * m_speed * deltaTime;
	if (dungeon.IsWall(newX - HALF_SIZE, m_position.z - HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, m_position.z - HALF_SIZE) ||
		dungeon.IsWall(newX - HALF_SIZE, m_position.z + HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, m_position.z + HALF_SIZE))
	{
		// 壁に当たった場合は X 位置を変えない（壁へのめり込みを防ぐ）
		newX = m_position.x;
	}

	// ---- Z 軸移動と壁衝突判定 ----
	// Z 判定には X で確定した newX を使う
	// これにより対角コーナーへの引っ掛かりを防ぎ、壁沿いのスライド移動が可能になる
	// X→Z の順に独立して解決するのが「軸分離 AABB スライド」の基本パターン
	float newZ = m_position.z + m_velVec.z * m_speed * deltaTime;
	if (dungeon.IsWall(newX - HALF_SIZE, newZ - HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, newZ - HALF_SIZE) ||
		dungeon.IsWall(newX - HALF_SIZE, newZ + HALF_SIZE) ||
		dungeon.IsWall(newX + HALF_SIZE, newZ + HALF_SIZE))
	{
		newZ = m_position.z;
	}

	m_position.x = newX;
	m_position.z = newZ;

	// 位置が変わったので bbox を再計算する
	m_bbox.setBBOX(m_position, m_scale);

	// プレイヤーから、到達可能なセルへの移動コストを計算
	CalcCostGrid(dungeon);
}

void Player::Draw(Renderer &renderer) const
{
	// キューブのピボットはモデル原点（底面中心）に合わせるため、
	// Translation の Y に PLAYER_CELL_SCALE * 0.5f を加算して床面から浮かせる
	XMMATRIX world = XMMatrixScaling(m_scale, m_scale, m_scale) * XMMatrixTranslation(m_position.x, m_position.y + PLAYER_CELL_SCALE * 0.5f, m_position.z);
	renderer.DrawCube(world, m_color);
}

void Player::hitEnemy()
{
	// 敵との衝突 1 回につき HP を 1 減らす
	// 0 以下になった場合の GameOver 判定は GameScene::Update() 側で行う
	m_hp--;

	// ダメージを受けたSEを鳴らす
	SoundManager::GetInstance().PlayHitPlayerSE();
}

void Player::CalcCostGrid(const Dungeon &dungeon)
{
	int gridSize = static_cast<int>(dungeon.getMazeSize());

	m_costGrid = std::vector<std::vector<int>>(gridSize, std::vector<int>(gridSize, -1));

	int playerGridX, playerGridZ;
	dungeon.WorldToGrid(m_position.x, m_position.z, playerGridX, playerGridZ);

	// BFSで、プレイヤーを始点として到達可能なマスへの最短経路を計算する
	std::queue<std::pair<int, std::pair<int, int>>> que;
	que.push({0, {playerGridZ, playerGridX}});

	std::vector<int> dz = {-1, 0, 1, 0};
	std::vector<int> dx = {0, 1, 0, -1};

	while (!que.empty())
	{
		auto [cost, pos] = que.front();
		que.pop();

		if (m_costGrid[pos.first][pos.second] != -1)
		{
			continue;
		}

		m_costGrid[pos.first][pos.second] = cost;

		for (int i = 0; i < 4; i++)
		{
			int nz = pos.first + dz[i];
			int nx = pos.second + dx[i];

			if (!(0 <= nz && nz < gridSize && 0 <= nx && nx < gridSize))
			{
				continue;
			}

			if (dungeon.GetGridType(nx, nz) == Dungeon::CellType::WALL)
			{
				continue;
			}

			que.push({cost + 1, {nz, nx}});
		}
	}
}
