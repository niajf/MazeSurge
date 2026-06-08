#include "MazeSurge/Dungeon.h"

Dungeon g_dungeon;

void Dungeon::GenerateTestMap()
{
	m_grid.push_back({START, WALL, WALL, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, WALL, WALL, WALL});
	m_grid.push_back({FLOOR, FLOOR, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, WALL, WALL, FLOOR, GOAL});
}

void Dungeon::Generate(unsigned seed)
{
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
	std::vector<int> di = {-2, 0, 2, 0};
	std::vector<int> dj = {0, 2, 0, -2};

	std::vector<int> order = {0, 1, 2, 3, 0, 1, 2, 3};

	while (!stk.empty())
	{
		auto [nowI, nowJ] = stk.top();

		int startIndex = rand() % 4;
		bool all = true;
		for (int k = 0; k < 4; k++)
		{
			int newI = nowI + di[order[startIndex + k]];
			int newJ = nowJ + dj[order[startIndex + k]];

			if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize))
				continue;

			if (m_grid[newI][newJ] == CellType::WALL)
			{
				m_grid[newI][newJ] = CellType::FLOOR;
				m_grid[nowI + di[order[startIndex + k]] / 2][nowJ + dj[order[startIndex + k]] / 2] = CellType::FLOOR;
				stk.push({newI, newJ});
				all = false;
				break;
			}
		}

		if (all)
			stk.pop();
	}
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
				XMMATRIX world = XMMatrixScaling(m_cellSize, m_cellSize, m_cellSize) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMFLOAT4 color = {1.0f, 1.0f, 0.0f, 1.0f};
				renderer.DrawCube(world, color);
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
		m_cellSize * (float)(gridX - (m_mazeSize - 1) / 2),
		0.0f,
		m_cellSize * (float)(gridZ - (m_mazeSize - 1) / 2)};
}

void Dungeon::WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const
{
	// GridToWorldの逆変換: worldX = m_cellSize * (gridX - (m_mazeSize-1)/2)
	// → gridX = round(worldX / m_cellSize + (m_mazeSize-1)/2)
	float gx = worldX / m_cellSize + (float)(m_mazeSize - 1) / 2.0f;
	float gz = worldZ / m_cellSize + (float)(m_mazeSize - 1) / 2.0f;
	gridX = std::clamp((int)std::round(gx), 0, m_mazeSize - 1);
	gridZ = std::clamp((int)std::round(gz), 0, m_mazeSize - 1);
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