#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Game/Projectile.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Utility/Collision.h"

class ProjectilePool
{
public:
    void Init(size_t poolSize = 50);
    Projectile *Get(XMFLOAT3 playerPos, XMFLOAT3 dir); // 非アクティブな弾を取得
    void Update(float deltaTime, const Dungeon &dungeon);
    void Draw(Renderer &renderer) const;
    bool DeactiveOnCollision(const BBOX &targetBbox);

private:
    std::vector<Projectile> m_pool; // 固定サイズ
};

extern ProjectilePool g_projectilePool;