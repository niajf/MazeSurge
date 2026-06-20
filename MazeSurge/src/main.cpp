#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Graphics/Renderer.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/ProjectilePool.h"
#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Scene/GameScene.h"
#include "MazeSurge/Scene/TitleScene.h"
#include <iostream>
#include <ctime>

// ---- 前方宣言 ----
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---- 入力構造体 ----
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

    // COM の初期化（WICTextureLoader が内部で使用する）
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"COMの初期化に失敗", L"エラー", MB_OK);
        return -1;
    }

    // ---- ウィンドウクラス登録 ----
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
    RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0, WINDOW_CLASS, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        MessageBox(nullptr, L"ウィンドウの作成に失敗", L"エラー", MB_OK);
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ---- 初期化 ----
    // std::unique_ptr<Scene> currentScene = std::make_unique<GameScene>();
    std::unique_ptr<Scene> currentScene = std::make_unique<TitleScene>();

    // ---- Rederer初期化 ----
    if (!g_renderer.Init(hwnd))
        return -1;

    // ---- シーン初期化 ----
    currentScene->Init();

    // ---- タイマー初期化 ----
    LARGE_INTEGER frequency, previousTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previousTime);

    // ---- ゲームループ ----
    MSG msg = {};
    bool isRunning = true;
    float fpsTimer = 0.0f;
    int frameCount = 0;

    while (isRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (isRunning)
        {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = static_cast<float>(currentTime.QuadPart - previousTime.QuadPart) / static_cast<float>(frequency.QuadPart);
            previousTime = currentTime;

            // 更新
            currentScene->Update(deltaTime, inputState);

            // 描画
            currentScene->Draw(g_renderer, inputState);

            if (currentScene->GetState() == GameState::Finished)
                DestroyWindow(hwnd);

            else if(currentScene->GetState()==GameState::Start)
            {
                currentScene = std::make_unique<GameScene>();
                currentScene->Init();
            }

            // FPS 表示
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

    CoUninitialize();
    return static_cast<int>(msg.wParam);
}

// ============================================================
// WndProc — ウィンドウプロシージャ
// ============================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
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
        inputState.mouseX = GET_X_LPARAM(lParam);
        inputState.mouseY = GET_Y_LPARAM(lParam);
        // {
        //     wchar_t buf[64];
        //     swprintf_s(buf, L"Mouse: (%d, %d)\n", inputState.mouseX, inputState.mouseY);
        //     OutputDebugStringW(buf);
        // }
        return 0;

    case WM_LBUTTONDOWN:
        inputState.lMouseDown = true;
        return 0;

    case WM_LBUTTONUP:
        inputState.lMouseDown = false;
        return 0;

    case WM_DESTROY:
        ClipCursor(nullptr);
        ShowCursor(TRUE);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}