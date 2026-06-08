#include "MazeSurge/Common.h"
#include "MazeSurge/Renderer.h"
#include "MazeSurge/Camera.h"
#include "MazeSurge/CubeManager.h"
#include "MazeSurge/Player.h"
#include "MazeSurge/Dungeon.h"
#include <iostream>

// ---- 前方宣言 ----
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
    RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
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
    if (!g_renderer.Init(hwnd)) return -1;
    g_dungeon.GenerateTestMap();
    g_player.Init(g_dungeon.GetStartPosition());
    g_camera.Init(14.0f, -7.0f);
    g_camera.Update(g_dungeon.GetStartPosition());

    // ---- タイマー初期化 ----
    LARGE_INTEGER frequency, previousTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previousTime);

    // ---- ゲームループ ----
    MSG msg = {};
    bool isRunning = true;
    float fpsTimer = 0.0f;
    int   frameCount = 0; 

    while (isRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { isRunning = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (isRunning)
        {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = static_cast<float>(currentTime.QuadPart - previousTime.QuadPart)
                / static_cast<float>(frequency.QuadPart);
            previousTime = currentTime;

            // 更新
            g_player.Update(deltaTime, g_dungeon);
            g_camera.Update(g_player.GetPosition());

            // 描画
            g_renderer.Render(deltaTime);
            g_player.Draw();
            g_dungeon.Draw(g_renderer);
            g_renderer.Present();

            // FPS 表示
            fpsTimer += deltaTime;
            frameCount++;
            if (fpsTimer >= 1.0f)
            {
                wchar_t title[64];
                swprintf_s(title, L"%s - FPS: %d, CubeScale : %d", WINDOW_TITLE, frameCount, g_cubeManager.getScale());
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
        case VK_ESCAPE: DestroyWindow(hwnd); break;
        case 'W': g_player.keyW = true; break;
        case 'A': g_player.keyA = true; break;
        case 'S': g_player.keyS = true; break;
        case 'D': g_player.keyD = true; break;
        case VK_UP:   g_cubeManager.keyUp = true;   break;
        case VK_DOWN: g_cubeManager.keyDown = true; break;
        }
        return 0;

    case WM_KEYUP:
        switch (wParam)
        {
        case 'W': g_player.keyW = false; break;
        case 'A': g_player.keyA = false; break;
        case 'S': g_player.keyS = false; break;
        case 'D': g_player.keyD = false; break;
        case VK_UP:   g_cubeManager.keyUp = false;   break;
        case VK_DOWN: g_cubeManager.keyDown = false; break;
        }
        return 0;

    case WM_DESTROY:
        ClipCursor(nullptr);
        ShowCursor(TRUE);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}