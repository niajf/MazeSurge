#include "MazeSurge/Scene/TitleScene.h"

void TitleScene::Init()
{
    m_state = GameState::Title;
    m_elapsedTime = 0.f;
}

void TitleScene::Update(float deltaTime, const InputState &inputState)
{
    m_elapsedTime += deltaTime;

    // シーン遷移直後の一定時間は入力を無効にする。
    // ゲームシーンでのボタン離し（lMouseUp）がそのままタイトルでも誤検出される問題を防ぐ。
    if (m_elapsedTime < INPUT_INVALID_TIME)
        return;

    // !lMouseDown && lMousePrevDown = クリック完了（Press → Release）の瞬間だけ処理。
    // 押しっぱなしや離し続けの状態は無視する。
    if (!inputState.lMouseDown && inputState.lMousePrevDown)
    {
        if (IsButtonClicked(inputState.mouseX, inputState.mouseY, TITLE_START_BUTTON_RECT))
            m_state = GameState::Start; // ゲームシーンへ遷移。

        if (IsButtonClicked(inputState.mouseX, inputState.mouseY, TITLE_EXIT_BUTTON_RECT))
            m_state = GameState::Finished; // アプリケーション終了。
    }
}

void TitleScene::Draw(Renderer &renderer, const InputState &inputState)
{
    // タイトル画面は 3D 描画なし。背景色でクリアしてから 2D スプライトを描画する。
    renderer.Clear(UI_TITLE_BG_COLOR.x, UI_TITLE_BG_COLOR.y, UI_TITLE_BG_COLOR.z);
    renderer.DrawTitle();
    renderer.Present();
}

bool TitleScene::IsButtonClicked(int x, int y, const RECT &button) const
{
    return (x >= button.left && x <= button.right && y >= button.top && y <= button.bottom);
}
