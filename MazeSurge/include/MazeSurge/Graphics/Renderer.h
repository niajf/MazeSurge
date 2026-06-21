#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Graphics/Cube.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/UI/UIConstant.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <CommonStates.h>
#include <WICTextureLoader.h>
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
    void Render(float floorScale, Camera &camera);

    // 外部から任意のキューブを1個描画する
    // Render()の後、Present()の前に呼ぶこと
    void DrawCube(const XMMATRIX &worldMatrix, const XMFLOAT4 &color);

    // 画面にプレイヤーの HP を描画する（Present()の前に呼ぶ）
    void DrawHP(int hp);

    // 画面にチェックポイント数を描画する（Present()の前に呼ぶ）
    void DrawCheckPoint(int getNum, size_t wholeNum);

    // 画面に残り時間を描画する（Present()の前に呼ぶ）
    void DrawTime(float time);

    // バックバッファを単色でクリアする（3Dシーンを使わない画面の先頭で呼ぶ）
    void Clear(float r = 0.1f, float g = 0.1f, float b = 0.15f);

    // タイトル画面を描画する（Clear()の後、Present()の前に呼ぶ）
    void DrawTitle();

    // 画面中央に "GAME OVER" とボタンを描画する（Present()の前に呼ぶ）
    void DrawGameOver(char rankChar);

    // 画面中央に "GAME CLEAR" とボタンを描画する（Present()の前に呼ぶ）
    void DrawGameClear(char rankChar);

    // 画面にプレイ方法を説明する図を描画する（Present()の前に呼ぶ）
    void DrawHowToPlay();

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

    // ---- 結果画面共通描画 ----
    void DrawResultScreen(const wchar_t *title, char rankChar, const XMFLOAT4 &titleColor, const XMFLOAT4 &btnColor);

    // ---- シェーダーコンパイルユーティリティ ----
    static bool CompileShader(const wchar_t *filePath, const char *entryPoint,
                              const char *profile, ComPtr<ID3DBlob> &blob);

    // ---- SpriteBatch / SpriteFont ----
    std::unique_ptr<SpriteBatch> m_spriteBatch;          // 2Dスプライト描画バッチ
    std::unique_ptr<SpriteFont> m_spriteFont;            // ビットマップフォント
    std::unique_ptr<CommonStates> m_states;              // ブレンド・ラスタライザ等の共通ステート
    ComPtr<ID3D11ShaderResourceView> m_whiteTexture;     // 単色描画用の1×1白テクスチャ
    ComPtr<ID3D11ShaderResourceView> m_titleTexture;     // タイトルロゴ画像
    ComPtr<ID3D11ShaderResourceView> m_titleBgTexture;   // タイトル背景画像
    ComPtr<ID3D11ShaderResourceView> m_howToPlayTexture; // プレイ方法説明画像
    UINT m_titleTexWidth = 0;                            // タイトルロゴのピクセル幅
    UINT m_titleTexHeight = 0;                           // タイトルロゴのピクセル高さ

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

    // ---- 行列 ----
    XMMATRIX m_view;       // ビュー行列（カメラ変換）
    XMMATRIX m_projection; // プロジェクション行列（透視投影）

    XMFLOAT4 m_floorColor;          // 床の描画色
    XMFLOAT4 m_playBackGroundColor; // プレイ中の背景クリア色
};

// グローバルレンダラーインスタンス
extern Renderer g_renderer;
