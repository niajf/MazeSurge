#include "MazeSurge/Game/EnemyManager.h"

static const float pi = 3.1415;

void EnemyManager::Init()
{
    m_poolSize = ENEMYMANAGER_POOL_SIZE;
    m_spawnTimer = 0.f;
    m_elapsedTime = 0.f;
    m_spawnInterval = ENEMYMANAGER_SWAWN_INTERVAL;

    for (size_t i = 0; i < m_poolSize; i++)
    {
        m_pool.emplace_back(Enemy());
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
            m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);

            return;
        }
    }
}

void EnemyManager::Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool)
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

        // 移動方向の変換
        XMFLOAT3 playerPosFloat = player.GetPosition();
        XMVECTOR playerPosVec = XMLoadFloat3(&playerPosFloat);
        XMVECTOR enemyPosVec = XMLoadFloat3(&m_pool[i].position);
        XMVECTOR dirVec = XMVectorSubtract(playerPosVec, enemyPosVec);
        XMVECTOR dirVecNorm = XMVector3Normalize(dirVec);
        XMFLOAT3 dirFloatNorm;
        XMStoreFloat3(&dirFloatNorm, dirVecNorm);

        // 移動先の計算
        m_pool[i].position.x += deltaTime * m_pool[i].speed * dirFloatNorm.x;
        m_pool[i].position.y += deltaTime * m_pool[i].speed * dirFloatNorm.y;
        m_pool[i].position.z += deltaTime * m_pool[i].speed * dirFloatNorm.z;
        m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);

        // プレイヤーと衝突したら、非活性化
        if (Collision::checkAABB(player.GetBBOX(), m_pool[i].bbox))
        {
            m_pool[i].active = false;
            player.hitEnemy();
        }

        // 球と衝突したら、非活性化
        if (projectilePool.DeactiveOnCollision(m_pool[i].bbox))
            m_pool[i].active = false;
    }
}

void EnemyManager::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        XMMATRIX world = XMMatrixScaling(m_pool[i].scale, m_pool[i].scale, m_pool[i].scale) * XMMatrixTranslation(m_pool[i].position.x, m_pool[i].position.y + 0.4f, m_pool[i].position.z);
        XMFLOAT4 projectileColor = {(float)128 / 255, (float)0 / 255, (float)0 / 255, 1.0f};
        renderer.DrawCube(world, projectileColor);
    }
}
