#include "MazeSurge/Game/EnemyManager.h"

static const float pi = 3.1415;

EnemyManager g_enemyManager;

void EnemyManager::Init(size_t poolSize)
{
    for (size_t i = 0; i < poolSize; i++)
    {
        m_pool.emplace_back(Enemy{});
    }
}

void EnemyManager::SpawnEnemy(Dungeon &dungeon)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
        {
            m_pool[i].active = true;

            float radian = pi * (rand() % 360) / 180.f;
            float norm = dungeon.getMazeSize() * dungeon.getCellSize();
            m_pool[i].position.x = norm * std::cos(radian);
            m_pool[i].position.y = 0.f;
            m_pool[i].position.z = norm * std::sin(radian);

            return;
        }
    }
}

void EnemyManager::Update(float deltaTime, const Player &player, Dungeon &dungeon)
{
    m_elapsedTime += deltaTime;

    // 時間経過で出現間隔を短縮（1分ごとに半分になる）
    m_spawnInterval = 3.0f / (1.0f + m_elapsedTime / 60.0f);

    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_spawnInterval)
    {
        m_spawnTimer = 0;
        SpawnEnemy(dungeon);
    }

    // 各敵のUpdates
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        XMFLOAT3 playerPosFloat = player.GetPosition();
        XMVECTOR playerPosVec = XMLoadFloat3(&playerPosFloat);
        XMVECTOR enemyPosVec = XMLoadFloat3(&m_pool[i].position);
        XMVECTOR dirVec = XMVectorSubtract(playerPosVec, enemyPosVec);
        XMVECTOR dirVecNorm = XMVector3Normalize(dirVec);
        XMFLOAT3 dirFloatNorm;
        XMStoreFloat3(&dirFloatNorm, dirVecNorm);

        m_pool[i].position.x += deltaTime * m_pool[i].speed * dirFloatNorm.x;
        m_pool[i].position.y += deltaTime * m_pool[i].speed * dirFloatNorm.y;
        m_pool[i].position.z += deltaTime * m_pool[i].speed * dirFloatNorm.z;
    }
}

void EnemyManager::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        XMMATRIX world = XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(m_pool[i].position.x, m_pool[i].position.y + 0.4f, m_pool[i].position.z);
        XMFLOAT4 projectileColor = {(float)128 / 255, (float)0 / 255, (float)0 / 255, 1.0f};
        renderer.DrawCube(world, projectileColor);
    }
}
