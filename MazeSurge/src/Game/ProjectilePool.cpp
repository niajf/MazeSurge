#include "MazeSurge/Game/ProjectilePool.h"

void ProjectilePool::Init()
{
    m_poolSize = PROJECTILEPOOL_POOL_SIZE;
    m_elapsedTime = 0.f;
    m_fireInterval = PROJECTILEPOOL_FIRE_INTERVAL_TIME;

    for (size_t i = 0; i < m_poolSize; i++)
        m_pool.emplace_back();
}

Projectile *ProjectilePool::Get(XMFLOAT3 playerPos, XMFLOAT3 dir)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
        {
            m_pool[i].active = true;
            m_pool[i].position = playerPos;
            m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);
            m_pool[i].direction = dir;
            m_pool[i].lifetime = 0.f;
            return &m_pool[i];
        }
    }

    return nullptr;
}

void ProjectilePool::Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &player, const InputState &inputState)
{
    m_elapsedTime += deltaTime;

    if (inputState.lMouseDown && m_elapsedTime > m_fireInterval)
    {
        m_elapsedTime = 0.0f;

        // planeY=-0.5f: プレイヤー中心(Y≈0)より半ユニット下の水平面にレイを当てる。
        // 弾の発射高さと一致させることで弾道が画面上のクリック位置と合う。
        XMFLOAT3 hitPos = camera.ScreenToWorldOnPlane(inputState.mouseX, inputState.mouseY, -0.5f);
        XMFLOAT3 playerPos = player.GetPosition();

        XMVECTOR dir_norm = XMVector3Normalize(
            XMVectorSubtract(XMLoadFloat3(&hitPos), XMLoadFloat3(&playerPos)));
        XMFLOAT3 dir_float3_norm;
        XMStoreFloat3(&dir_float3_norm, dir_norm);

        Get(playerPos, dir_float3_norm);
    }

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

        m_pool[i].position.x += deltaTime * m_pool[i].speed * m_pool[i].direction.x;
        m_pool[i].position.y += deltaTime * m_pool[i].speed * m_pool[i].direction.y;
        m_pool[i].position.z += deltaTime * m_pool[i].speed * m_pool[i].direction.z;
        m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);
    }
};

void ProjectilePool::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        XMMATRIX world = XMMatrixScaling(m_pool[i].scale, m_pool[i].scale, m_pool[i].scale) * XMMatrixTranslation(m_pool[i].position.x, m_pool[i].position.y + 0.5f, m_pool[i].position.z);
        renderer.DrawCube(world, m_pool[i].color);
    }
};

bool ProjectilePool::DeactiveOnCollision(const BBOX &targetBbox)
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        if (Collision::checkAABB(m_pool[i].bbox, targetBbox))
        {
            m_pool[i].active = false;
            return true;
        }
    }

    return false;
}