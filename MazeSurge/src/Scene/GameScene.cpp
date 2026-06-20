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
    m_timeLimit = TIME_LIMIT;
}

void GameScene::Update(float deltaTime, const InputState &inputState)
{
    if (m_state == GameState::Playing)
    {
        m_timeLimit -= deltaTime;

        if (m_timeLimit <= 0.f)
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
        if (inputState.lMouseDown && IsButtonClicked(inputState.mouseX, inputState.mouseY))
            m_state = GameState::Finished;
    }

    else if (m_state == GameState::GameOver)
    {
        if (inputState.lMouseDown && IsButtonClicked(inputState.mouseX, inputState.mouseY))
            m_state = GameState::Finished;
    }
}

void GameScene::Draw(Renderer &renderer, const InputState &inputState)
{
    if (m_state == GameState::Playing)
    {
        renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
        m_dungeon.Draw(renderer);
        m_projectilePool.Draw(renderer);
        m_enemyManager.Draw(renderer);
        m_player.Draw(renderer);
        renderer.DrawHP(m_player.GetHP());
        renderer.DrawCheckPoint(m_getCheckPoint, m_dungeon.GetRemainCheckPoint());
        renderer.DrawTime(m_timeLimit);
        renderer.Present();
    }

    else if (m_state == GameState::GameClear)
    {
        renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
        m_dungeon.Draw(renderer);
        m_projectilePool.Draw(renderer);
        m_enemyManager.Draw(renderer);
        m_player.Draw(renderer);
        renderer.DrawHP(m_player.GetHP());
        renderer.DrawCheckPoint(m_getCheckPoint, m_dungeon.GetRemainCheckPoint());
        renderer.DrawTime(m_timeLimit);
        renderer.DrawGameClear();
        renderer.Present();
    }

    else if (m_state == GameState::GameOver)
    {
        renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
        m_dungeon.Draw(renderer);
        m_projectilePool.Draw(renderer);
        m_enemyManager.Draw(renderer);
        m_player.Draw(renderer);
        renderer.DrawHP(m_player.GetHP());
        renderer.DrawCheckPoint(m_getCheckPoint, m_dungeon.GetRemainCheckPoint());
        renderer.DrawTime(m_timeLimit);
        renderer.DrawGameOver();
        renderer.Present();
    }
}

GameState GameScene::GetState()
{
    return m_state;
}

bool GameScene::IsButtonClicked(int x, int y) const
{
    return x >= GAME_OVER_BUTTON_RECT.left && x <= GAME_OVER_BUTTON_RECT.right &&
           y >= GAME_OVER_BUTTON_RECT.top && y <= GAME_OVER_BUTTON_RECT.bottom;
}