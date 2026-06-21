#include "MazeSurge/Scene/GameScene.h"

void GameScene::Init()
{
    m_state = GameState::Playing;
    m_dungeon.Generate(static_cast<unsigned int>(std::time(nullptr)));
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
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
            m_state = GameState::Finished;
    }

    else if (m_state == GameState::GameOver)
    {
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
            m_state = GameState::Finished;
    }
}

void GameScene::Draw(Renderer &renderer, const InputState &inputState)
{
    if (m_state == GameState::Playing)
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

        if (m_elapsedTime < HOW_TO_PLAY_DRAW_TIME)
        {
            renderer.DrawHowToPlay();
        }

        renderer.Present();
    }

    else if (m_state == GameState::GameClear)
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
        renderer.DrawGameClear(GetRankChar());
        renderer.Present();
    }

    else if (m_state == GameState::GameOver)
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
        renderer.DrawGameOver(GetRankChar());
        renderer.Present();
    }
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