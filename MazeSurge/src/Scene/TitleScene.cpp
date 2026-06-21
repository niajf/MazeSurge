#include "MazeSurge/Scene/TitleScene.h"

void TitleScene::Update(float deltaTime, const InputState &inputState)
{
    if (!inputState.lMouseDown && inputState.lMousePrevDown)
    {
        if (IsButtonClicked(inputState.mouseX, inputState.mouseY, TITLE_START_BUTTON_RECT))
        {
            m_state = GameState::Start;
        }

        if (IsButtonClicked(inputState.mouseX, inputState.mouseY, TITLE_EXIT_BUTTON_RECT))
        {
            m_state = GameState::Finished;
        }
    }
}

void TitleScene::Draw(Renderer &renderer, const InputState &inputState)
{
    renderer.Clear(UI_TITLE_BG_COLOR.x, UI_TITLE_BG_COLOR.y, UI_TITLE_BG_COLOR.z);
    renderer.DrawTitle();
    renderer.Present();
}

bool TitleScene::IsButtonClicked(int x, int y, const RECT &button) const
{
    return (x >= button.left && x <= button.right && y >= button.top && y <= button.bottom);
}