#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"
#include "MazeSurge/Graphics/Cube.h"

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

    // バックバッファを画面に表示する（ゲームループの最後に呼ぶ）
    void Present();

private:
    // ---- 初期化のサブ関数 ----
    bool CreateDeviceAndSwapChain(HWND hwnd);
    bool CreateRenderTargetAndDepthBuffer();
    bool CompileAndCreateShaders();
    bool CreateInputLayout(ComPtr<ID3DBlob> &vsBlob);
    bool CreateConstantBuffers();
    bool CreateMeshBuffers();

    // ---- シェーダーコンパイルユーティリティ ----
    static bool CompileShader(const wchar_t *filePath, const char *entryPoint,
                              const char *profile, ComPtr<ID3DBlob> &blob);

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