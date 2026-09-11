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
// Renderer — Direct3D 11 の初期化・リソース管理・描画を担う
// ============================================================
// D3D11 デバイス、スワップチェイン、シェーダー、バッファ、テクスチャなど
// 描画に必要なすべての GPU リソースをカプセル化する。
// グローバルインスタンス g_renderer（下部で extern 宣言）として使用する。
//
// 1 フレームの描画順序:
//   1. Clear()
//   2. Render()     — 床の 3D 描画、ビュー/プロジェクション行列のキャッシュ
//   3. DrawCube()   — 各オブジェクトのキューブ描画（複数回呼べる）
//   4. DrawHP() / DrawCheckPoint() / DrawTime() 等 — 2D HUD
//   5. Present()    — バックバッファをフロントに表示
class Renderer
{
public:
    // D3D11 の全リソース（デバイス・シェーダー・バッファ・テクスチャ等）を初期化する。
    // 失敗した場合は false を返す。
    bool Init(HWND hwnd);

    // 毎フレームの 3D 描画共通処理。
    // floorScale: 床の XZ スケール（迷路サイズに合わせて拡縮する）。
    // 内部でビュー/プロジェクション行列を m_view/m_projection にキャッシュするため、
    // DrawCube() より前に呼ぶこと。
    void Render(float floorScale, Camera &camera);

    // 任意のキューブを 1 個描画する。
    // Render() でキャッシュされた m_view/m_projection を使うため Render() 後に呼ぶこと。
    // worldMatrix: ワールド変換行列（スケール × 回転 × 平行移動）。
    void DrawCube(const XMMATRIX &worldMatrix, const XMFLOAT4 &color);

    // ---- 2D HUD 描画（SpriteBatch を使用）----
    // これらはすべて Present() の前に呼ぶこと。
    // SpriteBatch::Begin/End を内部で完結させているため、呼び出し順序は問わない。
    void DrawHP(int hp);                              // プレイヤーの HP バーを上部に表示
    void DrawCheckPoint(int getNum, size_t wholeNum); // 取得済み/全チェックポイント数を表示
    void DrawTime(float time);                        // 残り時間を MM:SS 形式で表示

    // バックバッファを単色でクリアする。
    // デフォルト引数は使用されないが、RENDERER_PLAY_BG_COLOR を明示的に渡すこと。
    void Clear(float r = 0.1f, float g = 0.1f, float b = 0.15f);

    // タイトル画面（背景・ロゴ・ボタン）を 2D スプライトで描画する。
    void DrawTitle();

    // ゲーム開始直後の操作説明画像を描画する。
    void DrawHowToPlay();

    // リザルト画面描画メソッド
    void DrawResultOverlay(float elapsedTime);
    void DrawResultText(const wchar_t *text, const XMFLOAT4 &textColor, float height);
    void DrawResultButton(const XMFLOAT4 &bntColor);

    // バックバッファをフロントバッファに表示する（垂直同期あり）。
    // ゲームループの最後に 1 度だけ呼ぶこと。
    void Present();

private:
    // ---- Init() のサブ関数（各フェーズを責務単位で分割）----
    bool CreateDeviceAndSwapChain(HWND hwnd);
    bool InitSpriteBatch(); // SpriteBatch・SpriteFont・テクスチャの初期化
    bool CreateRenderTargetAndDepthBuffer();
    bool CompileAndCreateShaders();
    bool CreateInputLayout(ComPtr<ID3DBlob> &vsBlob);
    bool CreateConstantBuffers();
    bool CreateMeshBuffers(); // キューブ・床の頂点/インデックスバッファを生成

    // HLSL ファイルをコンパイルしてバイトコード blob を返す。
    // static: インスタンス状態に依存しないため（m_device 等を使わない）。
    static bool CompileShader(const wchar_t *filePath, const char *entryPoint,
                              const char *profile, ComPtr<ID3DBlob> &blob);

    // ---- SpriteBatch / SpriteFont（DirectXTK）----
    std::unique_ptr<SpriteBatch> m_spriteBatch; // 2D スプライト描画バッチ
    std::unique_ptr<SpriteFont> m_spriteFont;   // ビットマップフォント
    std::unique_ptr<CommonStates> m_states;     // ブレンド・ラスタライザ等の共通ステート集

    // m_whiteTexture: 1×1 の不透明白ピクセルテクスチャ。
    // SpriteBatch::Draw がテクスチャを必須とするため、ベタ塗り矩形の描画に使う。
    // tint カラー引数で任意の色を指定できる。
    ComPtr<ID3D11ShaderResourceView> m_whiteTexture;
    ComPtr<ID3D11ShaderResourceView> m_titleTexture;     // タイトルロゴ画像
    ComPtr<ID3D11ShaderResourceView> m_titleBgTexture;   // タイトル背景画像
    ComPtr<ID3D11ShaderResourceView> m_howToPlayTexture; // 操作説明画像

    // ロゴをアスペクト比を保ったままスケーリングするために元のピクセルサイズを保持する。
    UINT m_titleTexWidth = 0;
    UINT m_titleTexHeight = 0;

    // ---- D3D11 コアオブジェクト ----
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    // ---- シェーダー ----
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;

    // m_inputLayout: Vertex 構造体のメモリレイアウトを D3D11 に伝えるオブジェクト。
    // VS バイトコードと Vertex 構造体の両方が変わった場合は再作成が必要。
    ComPtr<ID3D11InputLayout> m_inputLayout;

    // ---- バッファ ----
    ComPtr<ID3D11Buffer> m_vertexBuffer;      // キューブ頂点（24 頂点）
    ComPtr<ID3D11Buffer> m_indexBuffer;       // キューブインデックス（36 個）
    ComPtr<ID3D11Buffer> m_floorVertexBuffer; // 床頂点（4 頂点）
    ComPtr<ID3D11Buffer> m_floorIndexBuffer;  // 床インデックス（6 個）
    ComPtr<ID3D11Buffer> m_constantBuffer;    // WVP + ワールド行列（register b0）
    ComPtr<ID3D11Buffer> m_lightBuffer;       // ライトパラメータ（register b1）

    // ---- 行列キャッシュ ----
    // Render() で計算してキャッシュし、その後の DrawCube() 呼び出しで参照する。
    XMMATRIX m_view = XMMatrixIdentity();
    XMMATRIX m_projection = XMMatrixIdentity();

    XMFLOAT4 m_floorColor;          // 床の描画色
    XMFLOAT4 m_playBackGroundColor; // プレイ中の背景クリア色
};

// g_renderer: Renderer.cpp で定義されるグローバルシングルトン。
// main.cpp で Init() を呼び、各シーンの Draw() 内で使用する。
extern Renderer g_renderer;
