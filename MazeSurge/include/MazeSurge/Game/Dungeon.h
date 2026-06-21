#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <stack>
#include <queue>
#include <numeric>

class Dungeon
{
public:
    enum CellType
    {
        FLOOR,
        WALL,
        CHECKPOINT,
        GOAL,
        START
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
    float getCellSize() { return m_wallScale; };
    size_t GetCheckPointNum() { return m_numCheckPoint; };

private:
    XMFLOAT4 m_wallColor;
    XMFLOAT4 m_goalColor;
    XMFLOAT4 m_checlPointColor;
    std::vector<std::vector<CellType>> m_grid;
    int m_mazeSize;         // 迷路のサイズ
    size_t m_numCheckPoint; // チェックポイントの数
    float m_wallScale;      // 壁ブロックのスケール
    float m_goalScale;
    float m_checkPointScale;
};