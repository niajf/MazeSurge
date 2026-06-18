#include "MazeSurge/Game/ProjectilePool.h"

ProjectilePool g_projectilePool;

void ProjectilePool::Init(size_t poolSize)
{
    for (size_t i = 0; i < poolSize; i++)
    {
        m_pool.emplace_back(Projectile{});
    }
}

Projectile *ProjectilePool::Get(XMFLOAT3 playerPos, XMFLOAT3 dir)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
        {
            m_pool[i].active = true;
            m_pool[i].position = playerPos;
            m_pool[i].direction = dir;
            m_pool[i].lifetime = 0.f;
            return &m_pool[i];
        }
    }

    return nullptr;
}

void ProjectilePool::Update(float deltaTime, const Dungeon &dungeon)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        m_pool[i].lifetime += deltaTime;
        if (m_pool[i].lifetime > m_pool[i].maxLifetime)
        {
            m_pool[i].active = false;
            continue;
        }

        m_pool[i].position.x += deltaTime * m_pool[i].velocity * m_pool[i].direction.x;
        m_pool[i].position.y += deltaTime * m_pool[i].velocity * m_pool[i].direction.y;
        m_pool[i].position.z += deltaTime * m_pool[i].velocity * m_pool[i].direction.z;
    }
};

void ProjectilePool::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        XMMATRIX world = XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixTranslation(m_pool[i].position.x, m_pool[i].position.y + 0.5f, m_pool[i].position.z);
        XMFLOAT4 projectileColor = {(float)220 / 255, (float)20 / 255, (float)60 / 255, 1.0f};
        renderer.DrawCube(world, projectileColor);
    }
};