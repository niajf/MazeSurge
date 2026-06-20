#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Game/Projectile.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Utility/Collision.h"

class ProjectilePool
{
public:
    void Init(size_t poolSize = 50);
    Projectile *Get(XMFLOAT3 playerPos, XMFLOAT3 dir); // 非アクティブな弾を取得
    void Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &playe, const InputState &inputState);
    void Draw(Renderer &renderer) const;
    bool DeactiveOnCollision(const BBOX &targetBbox);

private:
    float m_fireInterval = 0.1f;
    float m_elapsedTime = 0.f;
    std::vector<Projectile> m_pool; // 固定サイズ
};