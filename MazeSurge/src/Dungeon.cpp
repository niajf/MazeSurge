#include "MazeSurge/Dungeon.h"
#include <cmath>
#include <algorithm>

Dungeon g_dungeon;

void Dungeon::GenerateTestMap()
{
	m_grid.push_back({ START, WALL,  WALL,  FLOOR, WALL });
	m_grid.push_back({ FLOOR, FLOOR, FLOOR, FLOOR, WALL });
	m_grid.push_back({ FLOOR, FLOOR, WALL,  WALL,  WALL });
	m_grid.push_back({ FLOOR, FLOOR, FLOOR, FLOOR, WALL });
	m_grid.push_back({ FLOOR, WALL,  WALL,  FLOOR, GOAL});
}

void Dungeon::Draw(Renderer& renderer) const
{
	for (int i = 0; i < m_width; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			if (m_grid[i][j] == WALL)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_cellSize, m_cellSize, m_cellSize) 
					* XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
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
	if (gridX < 0 || gridX >= m_width || gridZ < 0 || gridZ >= m_height)
		return true;
	return m_grid[gridZ][gridX] == WALL;
}

XMFLOAT3 Dungeon::GridToWorld(int gridX, int gridZ) const
{
	// グリッド中心座標を返す
	return 
	{ 
	m_cellSize * (float)(gridX - (m_width - 1) / 2),
	0.0f, 	
	m_cellSize * (float)(gridZ - (m_height - 1) / 2)
	};
}

void Dungeon::WorldToGrid(float worldX, float worldZ, int& gridX, int& gridZ) const
{
	// GridToWorldの逆変換: worldX = m_cellSize * (gridX - (m_width-1)/2)
	// → gridX = round(worldX / m_cellSize + (m_width-1)/2)
	float gx = worldX / m_cellSize + (float)(m_width - 1) / 2.0f;
	float gz = worldZ / m_cellSize + (float)(m_height - 1) / 2.0f;
	gridX = std::clamp((int)std::round(gx), 0, m_width - 1);
	gridZ = std::clamp((int)std::round(gz), 0, m_height - 1);
}

XMFLOAT3 Dungeon::GetStartPosition() const
{
	for (int i = 0; i < m_width; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			if (m_grid[i][j] == START)
			{
				return GridToWorld(j, i);
			}
		}
	}

	return { 0.0f, 0.0f, 0.0f };
}

XMFLOAT3 Dungeon::GetGoalPosition() const
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			if (m_grid[i][j] == GOAL)
				return GridToWorld(j, i);
		}
	}
	return { 0.0f, 0.0f, 0.0f };
}