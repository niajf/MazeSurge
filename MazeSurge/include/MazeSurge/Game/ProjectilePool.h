#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Projectile.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Utility/Collision.h"

class ProjectilePool
{
public:
    void Init();
    Projectile *Get(XMFLOAT3 playerPos, XMFLOAT3 dir); // 非アクティブな弾を取得
    void Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &playe, const InputState &inputState);
    void Draw(Renderer &renderer) const;
    bool DeactiveOnCollision(const BBOX &targetBbox);

private:
    std::vector<Projectile> m_pool; // 固定サイズ
    size_t m_poolSize;
    float m_fireInterval;
    float m_elapsedTime;
};