#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Scene/Scene.h"
#include "MazeSurge/Graphics/Renderer.h"

class TitleScene : public Scene
{
public:
    TitleScene() = default;
    void Init();
    void Update(float deltaTime, const InputState &inputState) override;
    void Draw(Renderer &renderer, const InputState &inputState) override;
    GameState GetState() { return m_state; };

private:
    bool IsButtonClicked(int x, int y, const RECT &button) const; // 指定座標がボタン矩形内にあるかを返す
    GameState m_state;                                            // 現在の状態（Title / Start / Finished 等、シーン遷移の判断に使用）
    float m_elapsedTime;
};