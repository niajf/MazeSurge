#pragma once
#include "MazeSurge/Common.h"
#include "MazeSurge/Renderer.h"
#include <vector>

class Dungeon
{
public:
    enum CellType { FLOOR = 0, WALL = 1, CHECKPOINT = 2, GOAL = 3, START = 4 };

    void GenerateTestMap();              // Week 1ではこれだけ
    void Generate(int width, int height, int seed); // Week 2で実装
    void Draw(Renderer& renderer) const;

    // 衝突判定用
    bool IsWall(float worldX, float worldZ) const;
    bool IsWallCell(int gridX, int gridZ) const;

    // 座標変換
    XMFLOAT3 GridToWorld(int gridX, int gridZ) const;
    void WorldToGrid(float worldX, float worldZ, int& gridX, int& gridZ) const;

    XMFLOAT3 GetStartPosition() const;
    XMFLOAT3 GetGoalPosition() const;

private:
    std::vector<std::vector<CellType>> m_grid;
    int m_width = 5;   // 奇数推奨（迷路アルゴリズムの都合）
    int m_height = 5;
    float m_cellSize = 2.0f; // 1セル = 2ワールド単位
};

extern Dungeon g_dungeon;