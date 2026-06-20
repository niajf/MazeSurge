#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Utility/Collision.h"
#include "MazeSurge/Game/Enemy.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/ProjectilePool.h"
#include <cstdlib>
#include <cmath>

class EnemyManager
{
public:
    void Init();
    void SpawnEnemy(Dungeon &dungeon);
    void Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool);
    void Draw(Renderer &rederer) const;

private:
    size_t m_poolSize;
    float m_spawnTimer;        // 敵生成に用いるタイマー（敵を生成してからの時間）
    float m_elapsedTime;       // 敵の生成間隔を調整に用いるタイマー（ゲームが開始してからの時間）
    float m_spawnInterval;     // 敵の生成間隔
    std::vector<Enemy> m_pool; // 敵の管理に用いるプール
};