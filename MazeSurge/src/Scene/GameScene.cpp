#include "MazeSurge/Scene/GameScene.h"

void GameScene::Init()
{
    m_state = GameState::Playing;
    m_dungeon.Init();
    m_player.Init(m_dungeon.GetStartPosition());
    m_camera.Init();
    m_projectilePool.Init();
    m_enemyManager.Init();

    m_getCheckPoint = 0;
    m_elapsedTime = 0.f;
    m_timeLimit = TIME_LIMIT;
}

void GameScene::Update(float deltaTime, const InputState &inputState)
{
    if (m_state == GameState::Playing)
    {
        m_elapsedTime += deltaTime;

        if (m_elapsedTime > m_timeLimit)
        {
            m_state = GameState::GameOver;
        }

        m_player.Update(deltaTime, m_dungeon, inputState);
        m_camera.Update(m_player.GetPosition());
        m_projectilePool.Update(deltaTime, m_camera, m_dungeon, m_player, inputState);
        m_enemyManager.Update(deltaTime, m_player, m_dungeon, m_projectilePool);

        if (m_dungeon.IsCheckPoint(m_player.GetPosition()))
            m_getCheckPoint++;

        if (m_dungeon.IsGoal(m_player.GetPosition()))
        {
            m_state = GameState::GameClear;
        }

        if (m_player.GetHP() <= 0)
        {
            m_state = GameState::GameOver;
        }
    }

    else if (m_state == GameState::GameClear)
    {
        // !lMouseDown && lMousePrevDown = ボタンを離した瞬間のみ反応（クリック完了判定）
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
            m_state = GameState::Restart;
    }

    else if (m_state == GameState::GameOver)
    {
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
            m_state = GameState::Restart;
    }
}

void GameScene::DrawScene3D(Renderer &renderer)
{
    renderer.Clear(RENDERER_PLAY_BG_COLOR.x, RENDERER_PLAY_BG_COLOR.y, RENDERER_PLAY_BG_COLOR.z);
    renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
    m_dungeon.Draw(renderer);
    m_projectilePool.Draw(renderer);
    m_enemyManager.Draw(renderer);
    m_player.Draw(renderer);
    renderer.DrawHP(m_player.GetHP());
    renderer.DrawCheckPoint(m_getCheckPoint, m_dungeon.GetCheckPointNum());
    renderer.DrawTime(m_timeLimit - m_elapsedTime);
}

void GameScene::Draw(Renderer &renderer, const InputState &inputState)
{
    if (m_state == GameState::Playing)
    {
        DrawScene3D(renderer);
        if (m_elapsedTime < HOW_TO_PLAY_DRAW_TIME)
            renderer.DrawHowToPlay();
    }
    else if (m_state == GameState::GameClear)
    {
        DrawScene3D(renderer);
        renderer.DrawGameClear(GetRankChar());
    }
    else if (m_state == GameState::GameOver)
    {
        DrawScene3D(renderer);
        renderer.DrawGameOver(GetRankChar());
    }

    renderer.Present();
}

GameState GameScene::GetState()
{
    return m_state;
}

bool GameScene::IsButtonClicked(int x, int y, const RECT &button) const
{
    return x >= button.left && x <= button.right &&
           y >= button.top && y <= button.bottom;
}

char GameScene::GetRankChar()
{

    if (m_state == GameState::GameOver)
        return 'D';

    // 残り時間比率とチェックポイント取得率の平均をスコアとする（各 0.0〜1.0）
    float timeScore = (m_timeLimit - m_elapsedTime) / m_timeLimit;
    float checkPointScore = static_cast<float>(m_getCheckPoint) / static_cast<float>(m_dungeon.GetCheckPointNum());
    float totalScore = (timeScore + checkPointScore) / 2.f;

    if (totalScore >= RANK_S_SCORE)
        return 'S';
    else if (totalScore >= RANK_A_SCORE)
        return 'A';
    else if (totalScore >= RANK_B_SCORE)
        return 'B';
    else if (totalScore >= RANK_C_SCORE)
        return 'C';

    return 'D';
}