#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"
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
    void Init(size_t poolSize = 50);
    void SpawnEnemy(Dungeon &dungeon);
    void Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool);
    void Draw(Renderer &rederer) const;

private:
    float m_spawnTimer = 0.0f;
    float m_spawnInterval = 3.0f; // 初期は3秒に1体
    float m_elapsedTime = 0.0f;
    std::vector<Enemy> m_pool;
};

extern EnemyManager g_enemyManager;