#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <functional>
#include <iostream>

// シーンの基底クラス（インターフェース）
enum class GameState
{
    Title,
    Start,
    Playing,
    GameOver,
    GameClear,
    Finished
};

class Scene
{
public:
    virtual void Init() = 0;
    virtual void Update(float deltaTIme, const InputState &inputState) = 0;
    virtual void Draw(Renderer &renderer, const InputState &inputState) = 0;
    virtual GameState GetState() = 0;
    virtual ~Scene() = default;
};