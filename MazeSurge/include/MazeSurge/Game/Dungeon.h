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

// ============================================================
// Dungeon — 迷路データの生成・管理・描画
// ============================================================
// グリッドは 2D ベクター（m_grid[行=Z][列=X]）で管理する。
// 迷路生成には再帰的バックトラッキング（穴掘り法）を使用し、
// BFS で最遠セルをゴールに、行き止まりをチェックポイントに割り当てる。
class Dungeon
{
public:
    // セルの種類。FLOOR が通路、WALL が壁。
    // 穴掘り法では全セルを WALL から始め、掘り進める過程で FLOOR に変える。
    enum CellType
    {
        FLOOR,      // 通路（プレイヤー・敵が移動可能）
        WALL,       // 壁（衝突判定あり・描画あり）
        CHECKPOINT, // チェックポイント（取得すると FLOOR に変わる）
        GOAL,       // ゴール（到達するとゲームクリア）
        START       // スタート地点（プレイヤーの初期位置）
    };

    // グリッドを確保してパラメータを初期化し、Generate() で迷路を生成する。
    void Init();
    // デバッグ用の固定マップを設定する（Generate() の代わりに使う）。
    void GenerateTestMap();
    // 再帰的バックトラッキング法でランダムな迷路を生成する。
    // 内部で BFS を使い、最遠セルをゴールに、行き止まりをチェックポイントに配置する。
    void Generate();
    // グリッドを走査し、WALL/GOAL/CHECKPOINT セルのみキューブとして描画する。
    void Draw(Renderer &renderer) const;

    // ---- 衝突判定 ----
    // ワールド座標をグリッドに変換して IsWallCell() に委譲する。
    bool IsWall(float worldX, float worldZ) const;
    // グリッド座標のセルが WALL かどうか（範囲外も true を返す）。
    bool IsWallCell(int gridX, int gridZ) const;

    // ---- 座標変換 ----
    // グリッド中心を原点とするワールド座標への線形変換。
    XMFLOAT3 GridToWorld(int gridX, int gridZ) const;
    // GridToWorld の逆変換。最近傍グリッドに丸めてクランプする。
    void WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const;

    // プレイヤーがチェックポイントセルにいれば true を返し、そのセルを FLOOR に変える。
    // セルを FLOOR に変えることで二重取得を防ぎ、視覚的にも消滅する。
    bool IsCheckPoint(XMFLOAT3 playerPos);
    // プレイヤーがゴールセルにいれば true を返す（セルは変化しない）。
    bool IsGoal(XMFLOAT3 playerPos);

    // スタートセルのワールド座標を返す（プレイヤー初期化時に使用）。
    XMFLOAT3 GetStartPosition() const;
    // ゴールセルのワールド座標を返す（現在は使用していないが配置確認に便利）。
    XMFLOAT3 GetGoalPosition() const;

    // 迷路のグリッドサイズ（縦横ともに同値）を返す。
    int getMazeSize() { return m_mazeSize; }
    // 1 セルのワールド単位サイズを返す（床スケール計算に使用）。
    float getCellSize() { return m_wallScale; }
    // チェックポイントの総数を返す（Generate() で確定した実際の数）。
    // DUNGEON_CHECKPOINT_NUM より少ない場合がある（行き止まり数の上限による）。
    size_t GetCheckPointNum() { return m_numCheckPoint; }

private:
    XMFLOAT4 m_wallColor;       // 壁セルの描画色
    XMFLOAT4 m_goalColor;       // ゴールセルの描画色
    XMFLOAT4 m_checlPointColor; // チェックポイントセルの描画色
    // m_grid は行優先（row-major）: m_grid[行=Z][列=X] でアクセスする。
    // GridToWorld / WorldToGrid と引数の順序（gridX, gridZ）が逆になるため注意。
    std::vector<std::vector<CellType>> m_grid;
    int m_mazeSize;          // 迷路のグリッドサイズ（奇数推奨）
    size_t m_numCheckPoint;  // 実際に配置されたチェックポイント数
    float m_wallScale;       // 壁ブロックのスケール（= 1 セルのワールドサイズ）
    float m_goalScale;       // ゴールブロックのスケール
    float m_checkPointScale; // チェックポイントブロックのスケール
};
