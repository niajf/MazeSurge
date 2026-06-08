#pragma once
#include "MazeSurge/Common.h"
#include "MazeSurge/Renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <stack>

class Dungeon
{
public:
    enum CellType
    {
        FLOOR = 0,
        WALL = 1,
        CHECKPOINT = 2,
        GOAL = 3,
        START = 4
    };

    void GenerateTestMap();
    void Generate(unsigned seed);
    void Draw(Renderer &renderer) const;

    // 衝突判定用
    bool IsWall(float worldX, float worldZ) const;
    bool IsWallCell(int gridX, int gridZ) const;

    // 座標変換
    XMFLOAT3 GridToWorld(int gridX, int gridZ) const;
    void WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const;

    bool IsCheckPoint(XMFLOAT3 playerPos);
    bool IsGoal(XMFLOAT3 playerPos);

    XMFLOAT3 GetStartPosition() const;
    XMFLOAT3 GetGoalPosition() const;
    int getMazeSize() { return m_mazeSize; };

private:
    std::vector<std::vector<CellType>> m_grid;
    int m_mazeSize = 5;      // 奇数
    float m_cellSize = 2.0f; // 1セル = 2ワールド単位
};

extern Dungeon g_dungeon;