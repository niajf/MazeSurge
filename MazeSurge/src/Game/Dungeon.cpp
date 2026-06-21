#include "MazeSurge/Game/Dungeon.h"

void Dungeon::GenerateTestMap()
{
	m_grid.push_back({START, WALL, WALL, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, WALL, WALL, WALL});
	m_grid.push_back({FLOOR, CHECKPOINT, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, WALL, WALL, FLOOR, GOAL});
}

void Dungeon::Generate(unsigned seed)
{
	// メンバ変数の初期化
	m_wallColor = DUNGEON_WALL_COLOR;
	m_goalColor = DUNGEON_GOAL_COLOR;
	m_checlPointColor = DUNGEON_CP_COLOR;
	m_mazeSize = DUNGEON_GRID_SIZE;
	m_numCheckPoint = DUNGEON_CHECKPOINT_NUM;
	m_wallScale = DUNGEON_WALL_SCALE;
	m_goalScale = DUNGEON_GOAL_SCALE;
	m_checkPointScale = DUNGEON_CP_SCALE;

	// シードを設定
	std::srand(seed);

	// 全てのマスを壁で埋める
	m_grid.resize(m_mazeSize, std::vector<CellType>(m_mazeSize, WALL));

	// スタート座標
	std::pair<int, int> startPos = {1, 1};

	// 迷路生成に用いるスタックの定義
	std::stack<std::pair<int, int>> stk;
	stk.push(startPos);
	m_grid[startPos.first][startPos.second] = CellType::START;

	// 遷移先の座標の差分を配列で定義
	std::vector<int> di = {-1, 0, 1, 0};
	std::vector<int> dj = {0, 1, 0, -1};

	// 迷路が単純にならないように、穴を掘る方向をランダムに決定するための配列
	std::vector<int> order = {0, 1, 2, 3, 0, 1, 2, 3};

	// 穴掘り法で迷路を生成
	while (!stk.empty())
	{
		auto [nowI, nowJ] = stk.top();

		int startIndex = rand() % 4;
		bool all = true;
		for (int k = 0; k < 4; k++)
		{
			int newI = nowI + di[order[startIndex + k]] * 2;
			int newJ = nowJ + dj[order[startIndex + k]] * 2;

			if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize))
				continue;

			if (m_grid[newI][newJ] == CellType::WALL)
			{
				m_grid[nowI + di[order[startIndex + k]]][nowJ + dj[order[startIndex + k]]] = CellType::FLOOR;
				m_grid[nowI + di[order[startIndex + k]] * 2][nowJ + dj[order[startIndex + k]] * 2] = CellType::FLOOR;
				stk.push({newI, newJ});
				all = false;
				break;
			}
		}

		if (all)
			stk.pop();
	}

	// スタートからのマンハッタン距離を保存する配列
	std::vector<std::vector<int>> distL1(m_mazeSize, std::vector<int>(m_mazeSize, -1));

	// 行き止まりのセルを保存する配列
	std::vector<std::pair<int, int>> deadEnds;

	// BFS用のキュー(col, row, dist)
	std::queue<std::tuple<int, int, int>> que;

	// スタートから最も遠いセルを記録し、ゴールにする。
	std::pair<int, int> goalGrid = startPos;
	int maxDist = -1;

	que.push({startPos.first, startPos.second, 0});

	// BFS開始
	while (!que.empty())
	{
		auto [nowI, nowJ, nowDist] = que.front();
		que.pop();

		if (distL1[nowI][nowJ] == -1)
		{
			distL1[nowI][nowJ] = nowDist;
			if (maxDist < nowDist)
			{
				maxDist = nowDist;
				goalGrid = {nowI, nowJ};
			}
		}
		else
			continue;

		int freeDirCount = 0;
		for (int k = 0; k < 4; k++)
		{
			int newI = nowI + di[k];
			int newJ = nowJ + dj[k];

			if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize))
				continue;

			if (m_grid[newI][newJ] == CellType::FLOOR)
			{
				freeDirCount++;
				que.push({newI, newJ, nowDist + 1});
			}
		}

		if (freeDirCount == 1)
			deadEnds.push_back({nowI, nowJ});
	}

	// 行き止まりの中から、ランダムでチェックポイントを設定
	// ２番目までの要素はスタートとそれに隣接するセルであるため、候補から除外する
	deadEnds.erase(deadEnds.begin(), deadEnds.begin() + 2);

	// 指定したチェックポイント数が、生成可能なチェックポイント数よりも大きくならないようにする
	m_numCheckPoint = std::min(m_numCheckPoint, deadEnds.size() - 1);

	// チェックポイントにするセルをランダムに決定
	std::vector<size_t> idxVector(m_numCheckPoint);
	std::iota(idxVector.begin(), idxVector.end(), 0);
	for (size_t i = 0; i < m_numCheckPoint; i++)
	{
		size_t idx = rand() % idxVector.size();
		size_t idxDeadEnd = idxVector[idx];
		m_grid[deadEnds[idxDeadEnd].first][deadEnds[idxDeadEnd].second] = CellType::CHECKPOINT;
		idxVector.erase(idxVector.begin() + idx);
	}

	// 最も遠いセルをゴールに設定
	m_grid[goalGrid.first][goalGrid.second] = CellType::GOAL;
}

void Dungeon::Draw(Renderer &renderer) const
{
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			if (m_grid[i][j] == WALL)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_wallScale, m_wallScale, m_wallScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_wallColor);
			}

			else if (m_grid[i][j] == GOAL)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_goalScale, m_goalScale, m_goalScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_goalColor);
			}

			else if (m_grid[i][j] == CHECKPOINT)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_checkPointScale, m_checkPointScale, m_checkPointScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_checlPointColor);
			}
		}
	}
}

bool Dungeon::IsWall(float worldX, float worldZ) const
{
	int gridX = 0, gridZ = 0;
	WorldToGrid(worldX, worldZ, gridX, gridZ);
	return IsWallCell(gridX, gridZ);
}

bool Dungeon::IsWallCell(int gridX, int gridZ) const
{
	if (gridX < 0 || gridX >= m_mazeSize || gridZ < 0 || gridZ >= m_mazeSize)
		return true;

	return m_grid[gridZ][gridX] == WALL;
}

XMFLOAT3 Dungeon::GridToWorld(int gridX, int gridZ) const
{
	// グリッド中心座標を返す
	return {
		m_wallScale * (float)(gridX - (m_mazeSize - 1) / 2),
		0.0f,
		m_wallScale * (float)(gridZ - (m_mazeSize - 1) / 2)};
}

void Dungeon::WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const
{
	// GridToWorldの逆変換: worldX = m_wallScale * (gridX - (m_mazeSize-1)/2)
	// → gridX = round(worldX / m_wallScale + (m_mazeSize-1)/2)
	float gx = worldX / m_wallScale + (float)(m_mazeSize - 1) / 2.0f;
	float gz = worldZ / m_wallScale + (float)(m_mazeSize - 1) / 2.0f;
	gridX = std::clamp((int)std::round(gx), 0, m_mazeSize - 1);
	gridZ = std::clamp((int)std::round(gz), 0, m_mazeSize - 1);
}

bool Dungeon::IsCheckPoint(XMFLOAT3 playerPos)
{
	int gridX = 0, gridZ = 0;
	WorldToGrid(playerPos.x, playerPos.z, gridX, gridZ);

	if (m_grid[gridZ][gridX] == CellType::CHECKPOINT)
	{
		// チェックポイントを取得した際の処理
		m_grid[gridZ][gridX] = CellType::FLOOR;
		return true;
	}

	return false;
}

bool Dungeon::IsGoal(XMFLOAT3 playerPos)
{
	int gridX = 0, gridZ = 0;
	WorldToGrid(playerPos.x, playerPos.z, gridX, gridZ);

	if (m_grid[gridZ][gridX] == CellType::GOAL)
		return true;

	return false;
}

XMFLOAT3 Dungeon::GetStartPosition() const
{
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			if (m_grid[i][j] == START)
			{
				return GridToWorld(j, i);
			}
		}
	}

	return {0.0f, 0.0f, 0.0f};
}

XMFLOAT3 Dungeon::GetGoalPosition() const
{
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			if (m_grid[i][j] == GOAL)
				return GridToWorld(j, i);
		}
	}
	return {0.0f, 0.0f, 0.0f};
}