#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Core/GameConstant.h"
#include "MazeSurge/Scene/Scene.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/ProjectilePool.h"

class GameScene : public Scene
{
public:
    GameScene() = default;
    void Init() override;
    void Update(float deltaTime, const InputState &inputState) override;
    void Draw(Renderer &renderer, const InputState &inputState) override;
    GameState GetState() override;

private:
    bool IsButtonClicked(int x, int y) const;

    Camera m_camera;
    Player m_player;
    Dungeon m_dungeon;
    EnemyManager m_enemyManager;
    ProjectilePool m_projectilePool;
    GameState m_state;

    int m_getCheckPoint;
    float m_timeLimit;
};