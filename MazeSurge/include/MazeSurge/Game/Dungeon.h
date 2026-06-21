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
#include <ctime>

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

    void Init();                          // グリッドを確保しパラメータを初期化してマップを生成する
    void GenerateTestMap();               // デバッグ用の固定マップを生成する
    void Generate();                      // ランダムな迷路を生成する（再帰的バックトラッキング法）
    void Draw(Renderer &renderer) const;  // グリッド上の全セルを描画する

    // 衝突判定用
    bool IsWall(float worldX, float worldZ) const;   // ワールド座標がグリッド上で壁かどうかを返す
    bool IsWallCell(int gridX, int gridZ) const;     // グリッド座標のセルが壁かどうかを返す

    // 座標変換
    XMFLOAT3 GridToWorld(int gridX, int gridZ) const;                      // グリッド座標をワールド座標に変換する
    void WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const; // ワールド座標をグリッド座標に変換する

    bool IsCheckPoint(XMFLOAT3 playerPos); // プレイヤーがチェックポイントセルにいれば true を返し、そのセルを FLOOR に変える
    bool IsGoal(XMFLOAT3 playerPos);       // プレイヤーがゴールセルにいれば true を返す

    XMFLOAT3 GetStartPosition() const; // スタートセルのワールド座標を返す
    XMFLOAT3 GetGoalPosition() const;  // ゴールセルのワールド座標を返す

    int getMazeSize() { return m_mazeSize; };         // 迷路のグリッドサイズを返す
    float getCellSize() { return m_wallScale; };      // 1セルあたりのワールド単位サイズを返す
    size_t GetCheckPointNum() { return m_numCheckPoint; }; // チェックポイントの総数を返す

private:
    XMFLOAT4 m_wallColor;                              // 壁セルの描画色
    XMFLOAT4 m_goalColor;                              // ゴールセルの描画色
    XMFLOAT4 m_checlPointColor;                        // チェックポイントセルの描画色
    std::vector<std::vector<CellType>> m_grid;         // 迷路の全セルを格納する 2D グリッド
    int m_mazeSize;                                    // 迷路のサイズ
    size_t m_numCheckPoint;                            // チェックポイントの数
    float m_wallScale;                                 // 壁ブロックのスケール（＝1セルのワールドサイズ）
    float m_goalScale;                                 // ゴールブロックのスケール
    float m_checkPointScale;                           // チェックポイントブロックのスケール
};