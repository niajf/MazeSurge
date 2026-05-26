#include "MazeSurge/Dungeon.h"
#include <cmath>
#include <algorithm>

Dungeon g_dungeon;

void Dungeon::GenerateTestMap()
{
	m_grid.push_back({ START, WALL, WALL, FLOOR, WALL });
	m_grid.push_back({ START, FLOOR, FLOOR, FLOOR, WALL });
	m_grid.push_back({ START, FLOOR, WALL, WALL, WALL });
	m_grid.push_back({ START, FLOOR, FLOOR, FLOOR, WALL });
	m_grid.push_back({ START, WALL, WALL, FLOOR, GOAL});
}

void Dungeon::Draw(Renderer& renderer) const
{
	for (int i = 0; i < m_width; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			if (m_grid[i][j] == WALL)
			{
				XMFLOAT3 pos = GridToWorld(i, j);
				XMMATRIX world = XMMatrixScaling(m_cellSize, m_cellSize, m_cellSize) 
					* XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
				renderer.DrawCube(world, color);
			}
		}
	}
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
	worldX = std::clamp(worldX, -m_cellSize * m_width, m_cellSize * m_width);
	worldX = std::clamp(worldZ, -m_cellSize * m_height, m_cellSize * m_height);
	gridX = std::floor(worldX / m_cellSize) + (m_width - 1) / 2;
	gridZ = std::floor(worldZ / m_cellSize) + (m_width - 1) / 2;
}