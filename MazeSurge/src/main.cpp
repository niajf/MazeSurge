#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Graphics/Renderer.h"
#include "MazeSurge/Audio/SoundManager.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/ProjectilePool.h"
#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Scene/GameScene.h"
#include "MazeSurge/Scene/TitleScene.h"
#include <iostream>

// ---- 前方宣言 ----
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// WndProc と WinMain で共有するグローバル入力状態。
// WndProc でウィンドウメッセージを受け取り、ゲームループで参照する。
InputState inputState = {};

// ============================================================
// WinMain — エントリポイント
// ============================================================
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // DirectXTK の WICTextureFromFile は内部で COM を使用する。
    // COINIT_MULTITHREADED: DirectX 系 API に推奨されるマルチスレッドアパートメント。
    // CoInitializeEx は WinMain の最初に呼び、CoUninitialize は終了直前に呼ぶ。
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"COMの初期化に失敗", L"エラー", MB_OK);
        return -1;
    }

    // ---- ウィンドウクラス登録 ----
    // CS_HREDRAW | CS_VREDRAW: ウィンドウサイズ変更時に全体を再描画する。
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"ウィンドウクラスの登録に失敗", L"エラー", MB_OK);
        return -1;
    }

    // ---- ウィンドウ作成 ----
    // WS_THICKFRAME（リサイズ枠）と WS_MAXIMIZEBOX（最大化ボタン）を除外して
    // ウィンドウサイズを固定する。D3D バックバッファサイズと常に一致させるため。
    constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    // AdjustWindowRect でタイトルバー・枠の分を含めた外寸を計算し、
    // クライアント領域が WINDOW_WIDTH × WINDOW_HEIGHT になるようにする。
    RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rc, WINDOW_STYLE, FALSE);

    HWND hwnd = CreateWindowEx(
        0, WINDOW_CLASS, WINDOW_TITLE, WINDOW_STYLE,
        CW_USEDEFAULT, CW_USEDEFAULT, // 初期位置は OS に任せる。
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        MessageBox(nullptr, L"ウィンドウの作成に失敗", L"エラー", MB_OK);
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd); // WM_PAINT を即座に送出してウィンドウを初期描画する。

    // XAudio2を初期化
    if (!g_soundManager.Init())
    {
        MessageBox(nullptr, L"XAudio2の初期化に失敗", L"エラー", MB_OK);
        return -1;
    }

    // BGMを鳴らす
    g_soundManager.PlayTitleBGM();

    // ---- 初期シーンをタイトルシーンとして生成 ----
    // unique_ptr<Scene> で多態性を持ちつつシーン切り替え時に自動解放する。
    std::unique_ptr<Scene> currentScene = std::make_unique<TitleScene>();

    // g_renderer はグローバルインスタンス（Renderer.cpp で定義）。
    if (!g_renderer.Init(hwnd))
        return -1;

    currentScene->Init();

    // ---- 高分解能タイマー初期化 ----
    // QueryPerformanceCounter は OS の高精度タイマーを使う。
    // timeGetTime (ms 精度) より精度が高く、deltaTime 計算に適している。
    LARGE_INTEGER frequency, previousTime;
    QueryPerformanceFrequency(&frequency);  // タイマーの周波数（カウント/秒）を取得。
    QueryPerformanceCounter(&previousTime); // ループ開始前の時刻を記録。

    // ---- ゲームループ ----
    MSG msg = {};
    bool isRunning = true;
    float fpsTimer = 0.0f;
    int frameCount = 0;

    while (isRunning)
    {
        // PeekMessage でメッセージキューを空にする。
        // GetMessage と異なりキューが空でもブロックしないため、描画ループが止まらない。
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                isRunning = false;
                break;
            }
            TranslateMessage(&msg); // WM_KEYDOWN → WM_CHAR へ変換（文字入力用）。
            DispatchMessage(&msg);  // WndProc にメッセージを転送する。
        }

        if (isRunning)
        {
            // ---- deltaTime の計算 ----
            // 前フレームからの経過時間（秒）を高精度タイマーで算出する。
            // deltaTime = (現在カウント - 前フレームカウント) / 周波数
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = static_cast<float>(currentTime.QuadPart - previousTime.QuadPart) / static_cast<float>(frequency.QuadPart);
            previousTime = currentTime;

            currentScene->Update(deltaTime, inputState);
            currentScene->Draw(g_renderer, inputState);

            // ---- シーン遷移ステートマシン ----
            if (currentScene->GetState() == GameState::Finished)
            {
                isRunning = false; // アプリケーション終了。
            }
            else if (currentScene->GetState() == GameState::Start)
            {
                // タイトル → ゲームシーン遷移。
                // unique_ptr の代入で旧シーンが自動解放されてから新シーンが初期化される。
                currentScene = std::make_unique<GameScene>();
                currentScene->Init();
                g_soundManager.StopBGM();
                g_soundManager.PlayGameBGM();
            }
            else if (currentScene->GetState() == GameState::Restart)
            {
                // ゲームオーバー/クリア → タイトルシーン遷移。
                currentScene = std::make_unique<TitleScene>();
                currentScene->Init();
                g_soundManager.StopBGM();
                g_soundManager.PlayTitleBGM();
            }

            // ---- FPS カウンターをタイトルバーに表示 ----
            // 1 秒ごとにフレーム数をカウントし、ウィンドウタイトルに反映する。
            fpsTimer += deltaTime;
            frameCount++;
            if (fpsTimer >= 1.0f)
            {
                wchar_t title[64];
                swprintf_s(title, L"%s - FPS: %d", WINDOW_TITLE, frameCount);
                SetWindowText(hwnd, title);
                fpsTimer = 0.0f;
                frameCount = 0;
            }
        }
    }

    // SoundManagerを解放
    g_soundManager.Cleanup();

    // COM を解放する。CoInitializeEx と対になる必要があるため、
    // D3D オブジェクトの解放（スコープ終了時）より後にならないよう注意する。
    CoUninitialize();
    return static_cast<int>(msg.wParam);
}

// ============================================================
// WndProc — ウィンドウプロシージャ
// ============================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // マウスの前フレーム状態を保存する。
    // これにより「ボタンを離した瞬間」（!lMouseDown && lMousePrevDown）を検出できる。
    // WndProc は毎メッセージ呼ばれるため、ゲームループの Update より前に評価される。
    inputState.lMousePrevDown = inputState.lMouseDown;

    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd); // ESC でウィンドウを閉じ、WM_DESTROY を発生させる。
            break;
        case 'W':
            inputState.keyW = true;
            break;
        case 'A':
            inputState.keyA = true;
            break;
        case 'S':
            inputState.keyS = true;
            break;
        case 'D':
            inputState.keyD = true;
            break;
        }
        return 0;

    case WM_KEYUP:
        switch (wParam)
        {
        case 'W':
            inputState.keyW = false;
            break;
        case 'A':
            inputState.keyA = false;
            break;
        case 'S':
            inputState.keyS = false;
            break;
        case 'D':
            inputState.keyD = false;
            break;
        }
        return 0;

    case WM_MOUSEMOVE:
        // GET_X_LPARAM / GET_Y_LPARAM はクライアント座標を取得するマクロ。
        // LPARAM をそのままキャストすると負値（ウィンドウ外）で誤動作する場合がある。
        inputState.mouseX = GET_X_LPARAM(lParam);
        inputState.mouseY = GET_Y_LPARAM(lParam);
        return 0;

    case WM_LBUTTONDOWN:
        inputState.lMouseDown = true;
        return 0;

    case WM_LBUTTONUP:
        inputState.lMouseDown = false;
        return 0;

    case WM_DESTROY:
        // カーソー制限（ClipCursor）を解除し、カーソーを再表示してから終了する。
        // ShowCursor(TRUE) で内部カウンタをインクリメントするため、
        // 非表示にした回数だけ呼ぶ必要がある（現在は 1 回非表示想定）。
        ClipCursor(nullptr);
        ShowCursor(TRUE);
        PostQuitMessage(0); // WM_QUIT をメッセージキューに積んでループを終了させる。
        return 0;
    }

    // 処理しないメッセージはデフォルトプロシージャに委譲する。
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
