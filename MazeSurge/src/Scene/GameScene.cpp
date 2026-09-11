#include "MazeSurge/Scene/GameScene.h"

void GameScene::Init()
{
    m_state = GameState::HowToPlay;

    // 各システムを依存関係の順に初期化する。
    // Dungeon を先に Init して GetStartPosition() を使えるようにしてから Player を Init する。
    m_dungeon.Init();
    m_player.Init(m_dungeon.GetStartPosition());
    m_camera.Init();
    m_projectilePool.Init();
    m_enemyManager.Init();

    m_getCheckPoint = 0;

    m_timeLimit = TIME_LIMIT;

    m_elapsedTime = 0.f;
    m_elapsedTimeHowToPlay = 0.f;
    m_elapsedTimePrepare = 0.f;
}

void GameScene::UpdateInGame(float deltaTime, const InputState &inputState)
{
    m_elapsedTime += deltaTime;

    // タイムリミット超過で GameOver に遷移。
    if (m_elapsedTime > m_timeLimit)
        m_state = GameState::GameOver;

    // 各システムを更新する順序は依存関係を考慮している。
    // Player → Camera → Projectile → Enemy の順で処理することで、
    // Camera がプレイヤー最新位置を参照でき、弾道計算にも反映される。
    m_player.Update(deltaTime, m_dungeon, inputState);
    m_camera.Update(m_player.GetPosition());
    m_projectilePool.Update(deltaTime, m_camera, m_dungeon, m_player, inputState);
    m_enemyManager.Update(deltaTime, m_player, m_dungeon, m_projectilePool);

    // IsCheckPoint はチェックポイントを踏むと FLOOR に書き換えて true を返す。
    // 同じチェックポイントを 2 度カウントしないようにグリッドを変更している。
    if (m_dungeon.IsCheckPoint(m_player.GetPosition()))
    {
        // チェックポイント取得数
        m_getCheckPoint++;

        // SE
        SoundManager::GetInstance().PlayGetCpSE();
    }

    if (m_dungeon.IsGoal(m_player.GetPosition()))
    {
        // BGM
        SoundManager::GetInstance().PlayClearBGM();

        // シーンフラグの変更
        m_state = GameState::GameClear;
        m_ResultTimer = 0.f;
        m_ResultSETimer = 0.f;
        m_rankUpCount = 0;
    }

    if (m_player.GetHP() <= 0)
    {
        // BGM
        SoundManager::GetInstance().PlayOverBGM();

        // シーンフラグの変更
        m_state = GameState::GameOver;
        m_ResultTimer = 0.f;
        m_ResultSETimer = 0.f;
        m_rankUpCount = 0;
    }
}

void GameScene::Update(float deltaTime, const InputState &inputState)
{

    if (m_state == GameState::HowToPlay)
    {
        m_elapsedTimeHowToPlay += deltaTime;

        // クリックされたらHow To Playを消し、ゲームを開始
        if ((!inputState.lMouseDown && inputState.lMousePrevDown) && m_elapsedTimeHowToPlay > 1.0f)
        {
            // 準備時間へ
            m_state = GameState::Prepare;

            // タイマーリセット
            m_elapsedTimeHowToPlay = 0.f;

            // SE
            SoundManager::GetInstance().PlaySelectSE();
        }

        m_camera.Update(m_player.GetPosition());
    }

    else if (m_state == GameState::Prepare)
    {
        m_elapsedTimePrepare += deltaTime;

        if (m_elapsedTimePrepare > 1.0f)
        {
            // インゲーム開始
            m_state = GameState::Playing;

            // タイマーリセット
            m_elapsedTimePrepare = 0.f;
        }

        m_camera.Update(m_player.GetPosition());
    }

    else if (m_state == GameState::Playing)
    {
        UpdateInGame(deltaTime, inputState);
    }

    else if (m_state == GameState::GameClear)
    {
        // !lMouseDown && lMousePrevDown = ボタンを「離した瞬間」のみ反応。
        // 押し続けている間は無視し、クリック完了（Press → Release）を検出する。
        // これによりシーン遷移直後の意図しない入力を防ぐ。
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
        {
            // リスタート
            m_state = GameState::Restart;

            // SE
            SoundManager::GetInstance().PlaySelectSE();
        }

        m_ResultTimer += deltaTime;
        m_ResultSETimer += deltaTime;

        if (m_ResultSETimer >= 3.0f)
        {
            m_ResultSETimer = 3.0f - UI_RESULT_DRAW_RANK_INREVAL;
            m_rankUpCount++;

            // SE
            if (m_rankUpCount <= 5)
                SoundManager::GetInstance().PlayRankUpSE();
        }
    }

    else if (m_state == GameState::GameOver)
    {
        if (!inputState.lMouseDown && inputState.lMousePrevDown && IsButtonClicked(inputState.mouseX, inputState.mouseY, GAME_EXIT_BUTTON_RECT))
        {
            // リスタート
            m_state = GameState::Restart;

            // SE
            SoundManager::GetInstance().PlaySelectSE();
        }

        m_ResultTimer += deltaTime;
        m_ResultSETimer += deltaTime;
    }
}

void GameScene::DrawScene3D(Renderer &renderer)
{
    // Clear → Render（床）→ 各オブジェクト → HUD の順で描画する。
    // HUD は SpriteBatch を使うため 3D 描画の後に呼ぶ必要がある。
    renderer.Clear(RENDERER_PLAY_BG_COLOR.x, RENDERER_PLAY_BG_COLOR.y, RENDERER_PLAY_BG_COLOR.z);

    // floorScale に迷路サイズを渡すことで床が迷路全体を覆う大きさになる。
    renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
    m_dungeon.Draw(renderer);
    m_projectilePool.Draw(renderer);
    m_enemyManager.Draw(renderer);
    m_player.Draw(renderer);

    // HUD 表示（2D スプライト）: 残り時間を表示するため timeLimit - elapsed を渡す。
    renderer.DrawHP(m_player.GetHP());
    renderer.DrawCheckPoint(m_getCheckPoint, m_dungeon.GetCheckPointNum());
    renderer.DrawTime(m_timeLimit - m_elapsedTime);
}

void GameScene::DrawScene3DPrePare(Renderer &renderer)
{
    // Clear → Render（床）→ 各オブジェクト → HUD の順で描画する。
    // HUD は SpriteBatch を使うため 3D 描画の後に呼ぶ必要がある。
    renderer.Clear(RENDERER_PLAY_BG_COLOR.x, RENDERER_PLAY_BG_COLOR.y, RENDERER_PLAY_BG_COLOR.z);

    // floorScale に迷路サイズを渡すことで床が迷路全体を覆う大きさになる。
    renderer.Render(static_cast<float>(m_dungeon.getMazeSize()), m_camera);
    m_dungeon.Draw(renderer);
    m_projectilePool.Draw(renderer);
    m_enemyManager.Draw(renderer);
}

void GameScene::DrawResultRank(Renderer &renderer)
{
    if (m_ResultTimer > 3.0f + UI_RESULT_DRAW_RANK_INREVAL * 5.f && GetRankInt() >= 5)
        renderer.DrawResultText(L"S", UI_RESULT_RANK_S_COLOR, 360.f);

    else if (m_ResultTimer > 3.0f + UI_RESULT_DRAW_RANK_INREVAL * 4.f && GetRankInt() >= 4)
        renderer.DrawResultText(L"A", UI_RESULT_RANK_A_COLOR, 360.f);

    else if (m_ResultTimer > 3.0f + UI_RESULT_DRAW_RANK_INREVAL * 3.f && GetRankInt() >= 3)
        renderer.DrawResultText(L"B", UI_RESULT_RANK_B_COLOR, 360.f);

    else if (m_ResultTimer > 3.0f + UI_RESULT_DRAW_RANK_INREVAL * 2.f && GetRankInt() >= 2)
        renderer.DrawResultText(L"C", UI_RESULT_RANK_C_COLOR, 360.f);

    else if (m_ResultTimer > 3.0f + UI_RESULT_DRAW_RANK_INREVAL && GetRankInt() >= 1)
        renderer.DrawResultText(L"D", UI_RESULT_RANK_D_COLOR, 360.f);
}

void GameScene::Draw(Renderer &renderer, const InputState &inputState)
{

    // ゲームシーン開始直後はHow To Playを描画する
    if (m_state == GameState::HowToPlay)
    {
        DrawScene3DPrePare(renderer);
        renderer.DrawHowToPlay();
    }

    else if (m_state == GameState::Prepare)
    {
        DrawScene3DPrePare(renderer);
    }

    else if (m_state == GameState::Playing)
    {
        DrawScene3D(renderer);
    }

    else if (m_state == GameState::GameClear)
    {
        // リザルト画面は 3D シーンの上にオーバーレイ表示する。
        DrawScene3D(renderer);
        renderer.DrawResultOverlay(m_ResultTimer);
        DrawResultRank(renderer);

        if (m_ResultTimer > UI_RESULT_DRAW_TITLE_SEC)
            renderer.DrawResultText(L"GAME CLEAR", UI_GAME_CLEAR_TITLE_COLOR, UI_RESULT_TITLE_Y);

        if (m_ResultTimer > UI_RESULT_DRAW_MIDLE_SEC)
            renderer.DrawResultText(L"SYNCHRO RANK", UI_RESULT_RANK_S_COLOR, UI_RESULT_MIDLE_Y);

        if (m_ResultTimer > UI_RESULT_DRAW_BTN_SEC + UI_RESULT_DRAW_RANK_INREVAL * 5.f)
            renderer.DrawResultButton(UI_GAME_CLEAR_BTN_COLOR);
    }

    else if (m_state == GameState::GameOver)
    {
        DrawScene3D(renderer);
        renderer.DrawResultOverlay(m_ResultTimer);
        DrawResultRank(renderer);

        if (m_ResultTimer > UI_RESULT_DRAW_TITLE_SEC)
            renderer.DrawResultText(L"GAME OVER", UI_GAME_OVER_TITLE_COLOR, UI_RESULT_TITLE_Y);

        if (m_ResultTimer > UI_RESULT_DRAW_MIDLE_SEC)
            renderer.DrawResultText(L"SYNCHRO RANK", UI_RESULT_RANK_S_COLOR, UI_RESULT_MIDLE_Y);

        if (m_ResultTimer > UI_RESULT_DRAW_BTN_SEC + UI_RESULT_DRAW_RANK_INREVAL * 5.f)
            renderer.DrawResultButton(UI_GAME_OVER_BTN_COLOR);
    }

    // Present は必ず最後に 1 度だけ呼ぶ。状態に関わらずここで統一する。
    renderer.Present();
}

GameState GameScene::GetState()
{
    return m_state;
}

bool GameScene::IsButtonClicked(int x, int y, const RECT &button) const
{
    // マウス座標がボタン矩形の内側かどうかを確認する（包含判定）。
    return x >= button.left && x <= button.right &&
           y >= button.top && y <= button.bottom;
}

int GameScene::GetRankInt()
{
    // GameOver 時は問答無用で最低ランク D。
    if (m_state == GameState::GameOver)
        return 1;

    // ランク計算: 残り時間比率とチェックポイント取得率の平均（各 0.0〜1.0）。
    // どちらかだけが高くても S ランクは取れないよう平均を採用している。
    float timeScore = (m_timeLimit - m_elapsedTime) / m_timeLimit;
    float checkPointScore = static_cast<float>(m_getCheckPoint) / static_cast<float>(m_dungeon.GetCheckPointNum());
    float totalScore = (timeScore + checkPointScore) / 2.f;

    if (totalScore >= RANK_S_SCORE)
        return 5;
    else if (totalScore >= RANK_A_SCORE)
        return 4;
    else if (totalScore >= RANK_B_SCORE)
        return 3;
    else if (totalScore >= RANK_C_SCORE)
        return 2;

    return 1;
}
