#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"
#include "MazeSurge/Graphics/Cube.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <memory>

// ============================================================
// Renderer — Direct3D 11 の初期化・リソース管理・描画
// ============================================================
// D3D11デバイス、スワップチェイン、シェーダー、バッファ、テクスチャなど
// 描画に必要なすべてのGPUリソースを管理する。
class Renderer
{
public:
    // D3D11の初期化（デバイス、スワップチェイン、シェーダー、バッファ等を一括作成）
    bool Init(HWND hwnd);

    // 毎フレームの描画処理
    void Render(float deltaTime, float floorScale);

    // 外部から任意のキューブを1個描画する
    // Render()の後、Present()の前に呼ぶこと
    void DrawCube(const XMMATRIX &worldMatrix, const XMFLOAT4 &color);

    // 画面左上にプレイヤーの HP を描画する（Present()の前に呼ぶ）
    void DrawHP(int hp);

    // 画面中央に "GAME OVER" とボタンを描画する（Present()の前に呼ぶ）
    void DrawGameOver();

    // 画面中央に "GAME CLEAR" とボタンを描画する（Present()の前に呼ぶ）
    void DrawGameClear();

    // 座標がゲームオーバーボタン内かどうか判定する
    bool IsGameOverButtonClicked(int x, int y) const;

    // バックバッファを画面に表示する（ゲームループの最後に呼ぶ）
    void Present();

private:
    // ---- 初期化のサブ関数 ----
    bool CreateDeviceAndSwapChain(HWND hwnd);
    bool InitSpriteBatch();
    bool CreateRenderTargetAndDepthBuffer();
    bool CompileAndCreateShaders();
    bool CreateInputLayout(ComPtr<ID3DBlob> &vsBlob);
    bool CreateConstantBuffers();
    bool CreateMeshBuffers();

    // ---- シェーダーコンパイルユーティリティ ----
    static bool CompileShader(const wchar_t *filePath, const char *entryPoint,
                              const char *profile, ComPtr<ID3DBlob> &blob);

    // ---- SpriteBatch / SpriteFont ----
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
    ComPtr<ID3D11ShaderResourceView> m_whiteTexture;
    RECT m_gameOverButtonRect = {};

    // ---- D3D11 コアオブジェクト ----
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    // ---- シェーダー ----
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;

    // ---- バッファ ----
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11Buffer> m_floorVertexBuffer;
    ComPtr<ID3D11Buffer> m_floorIndexBuffer;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11Buffer> m_lightBuffer;
};

// グローバルレンダラーインスタンス
extern Renderer g_renderer;