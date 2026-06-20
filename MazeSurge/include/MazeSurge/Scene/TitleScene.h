#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Scene/Scene.h"

class TitleScene : public Scene
{
public:
    TitleScene() = default;
    void Init() {};
    void Update(float deltaTime, const InputState &inputState) override;
    void Draw(Renderer &renderer, const InputState &inputState) override;
    GameState GetState() { return m_state; };

private:
    bool IsButtonClicked(int x, int y, const RECT &button) const;
    GameState m_state;
};