#include "MazeSurge/Graphics/Renderer.h"
#include "MazeSurge/Graphics/Camera.h"
#include <string>

// グローバルレンダラーインスタンスの定義
Renderer g_renderer;

// ============================================================
// Init — D3D11の初期化を一括で行う
// ============================================================
bool Renderer::Init(HWND hwnd)
{
    if (!CreateDeviceAndSwapChain(hwnd))
        return false;
    if (!CreateRenderTargetAndDepthBuffer())
        return false;

    // ビューポートの設定
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(WINDOW_WIDTH);
    viewport.Height = static_cast<float>(WINDOW_HEIGHT);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);

    if (!CompileAndCreateShaders())
        return false;
    if (!CreateConstantBuffers())
        return false;
    if (!CreateMeshBuffers())
        return false;
    if (!InitSpriteBatch())
        return false;

    m_floorColor = RENDERER_FLOOR_COLOR;
    m_playBackGroundColor = RENDERER_PLAY_BG_COLOR;

    return true;
}

// ============================================================
// デバイスとスワップチェインの作成
// ============================================================
bool Renderer::CreateDeviceAndSwapChain(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferDesc.Width = WINDOW_WIDTH;
    scd.BufferDesc.Height = WINDOW_HEIGHT;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = hwnd;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL featureLevelOut;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION, &scd,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &featureLevelOut,
        m_deviceContext.GetAddressOf());

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"D3D11CreateDeviceAndSwapChain に失敗", L"エラー", MB_OK);
        return false;
    }
    return true;
}

// ============================================================
// レンダーターゲットと深度バッファの作成
// ============================================================
bool Renderer::CreateRenderTargetAndDepthBuffer()
{
    // レンダーターゲットビュー
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                        reinterpret_cast<void **>(backBuffer.GetAddressOf()));
    if (FAILED(hr))
        return false;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                          m_renderTargetView.GetAddressOf());
    if (FAILED(hr))
        return false;

    // 深度バッファ
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = WINDOW_WIDTH;
    depthDesc.Height = WINDOW_HEIGHT;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthTexture;
    hr = m_device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_device->CreateDepthStencilView(depthTexture.Get(), nullptr,
                                          m_depthStencilView.GetAddressOf());
    if (FAILED(hr))
        return false;

    return true;
}

// ============================================================
// シェーダーコンパイルユーティリティ
// ============================================================
bool Renderer::CompileShader(
    const wchar_t *filePath, const char *entryPoint,
    const char *profile, ComPtr<ID3DBlob> &blob)
{
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint, profile, compileFlags, 0,
        blob.GetAddressOf(), errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        if (errorBlob)
            OutputDebugStringA(static_cast<const char *>(errorBlob->GetBufferPointer()));
        return false;
    }
    return true;
}

// ============================================================
// シェーダーのコンパイルと生成
// ============================================================
bool Renderer::CompileAndCreateShaders()
{
    // 頂点シェーダー
    ComPtr<ID3DBlob> vsBlob;
    if (!CompileShader(L"shaders/shaders.hlsl", "vs_main", "vs_5_0", vsBlob))
    {
        MessageBox(nullptr, L"頂点シェーダーのコンパイルに失敗", L"エラー", MB_OK);
        return false;
    }
    HRESULT hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr))
        return false;

    // ピクセルシェーダー
    ComPtr<ID3DBlob> psBlob;
    if (!CompileShader(L"shaders/shaders.hlsl", "ps_main", "ps_5_0", psBlob))
    {
        MessageBox(nullptr, L"ピクセルシェーダーのコンパイルに失敗", L"エラー", MB_OK);
        return false;
    }
    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr))
        return false;

    // 入力レイアウト
    if (!CreateInputLayout(vsBlob))
        return false;

    return true;
}

// ============================================================
// 入力レイアウトの作成
// ============================================================
bool Renderer::CreateInputLayout(ComPtr<ID3DBlob> &vsBlob)
{
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

    HRESULT hr = m_device->CreateInputLayout(
        layoutDesc, _countof(layoutDesc),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf());

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"入力レイアウトの作成に失敗", L"エラー", MB_OK);
        return false;
    }
    return true;
}

// ============================================================
// 定数バッファの作成
// ============================================================
bool Renderer::CreateConstantBuffers()
{
    // WVP用
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr = m_device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    // ライト用
    D3D11_BUFFER_DESC lbDesc = {};
    lbDesc.Usage = D3D11_USAGE_DEFAULT;
    lbDesc.ByteWidth = sizeof(LightBuffer);
    lbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = m_device->CreateBuffer(&lbDesc, nullptr, m_lightBuffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    return true;
}

// ============================================================
// メッシュ（頂点・インデックス）バッファの作成
// ============================================================
bool Renderer::CreateMeshBuffers()
{
    // ---- キューブ頂点データ（24頂点 = 6面 × 4頂点）----
    Vertex cubeVertices[] =
        {
            // 前面 (0,0,-1)
            {XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0)},
            {XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 0)},
            {XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 1)},
            {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 1)},
            // 背面 (0,0,1)
            {XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 0)},
            {XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 0)},
            {XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 1)},
            {XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 1)},
            // 上面 (0,1,0)
            {XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 0)},
            {XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 1)},
            {XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 1)},
            // 底面 (0,-1,0)
            {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 0)},
            {XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 1)},
            {XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 1)},
            // 左面 (-1,0,0)
            {XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 0)},
            {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 1)},
            {XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 1)},
            // 右面 (1,0,0)
            {XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 0)},
            {XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 1)},
            {XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 1)},
        };

    UINT cubeIndices[] =
        {
            0,
            1,
            2,
            0,
            2,
            3,
            4,
            5,
            6,
            4,
            6,
            7,
            8,
            9,
            10,
            8,
            10,
            11,
            12,
            13,
            14,
            12,
            14,
            15,
            16,
            17,
            18,
            16,
            18,
            19,
            20,
            21,
            22,
            20,
            22,
            23,
        };

    // ---- 床頂点データ ----
    Vertex floorVertices[] =
        {
            {XMFLOAT3(-1.f, -0.5f, -1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(1.f, -0.5f, -1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(10, 0)},
            {XMFLOAT3(1.f, -0.5f, 1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(10, 10)},
            {XMFLOAT3(-1.f, -0.5f, 1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 10)},
        };

    UINT floorIndices[] = {0, 2, 1, 0, 3, 2};

    // ---- バッファ作成ヘルパー ----
    auto createBuffer = [&](auto *data, UINT byteWidth, UINT bindFlags, ComPtr<ID3D11Buffer> &buffer)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = byteWidth;
        desc.BindFlags = bindFlags;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;

        return m_device->CreateBuffer(&desc, &initData, buffer.GetAddressOf());
    };

    HRESULT hr;
    hr = createBuffer(cubeVertices, sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer);
    if (FAILED(hr))
        return false;
    hr = createBuffer(cubeIndices, sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER, m_indexBuffer);
    if (FAILED(hr))
        return false;
    hr = createBuffer(floorVertices, sizeof(floorVertices), D3D11_BIND_VERTEX_BUFFER, m_floorVertexBuffer);
    if (FAILED(hr))
        return false;
    hr = createBuffer(floorIndices, sizeof(floorIndices), D3D11_BIND_INDEX_BUFFER, m_floorIndexBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

// ============================================================
// Render — 毎フレームの描画処理
// ============================================================
void Renderer::Render(float floorScale, Camera &camera)
{
    // SpriteBatch は深度・ブレンド・ラスタライザステートを内部で変更する。
    // nullptr を渡すと D3D11 デフォルト設定に戻り、次の 3D パスが正しく描画される。
    m_deviceContext->OMSetDepthStencilState(nullptr, 0);
    m_deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    m_deviceContext->RSSetState(nullptr);

    // ---- ビュー・プロジェクション行列をメンバにキャッシュ ----
    m_view = camera.GetViewMatrix();
    m_projection = camera.GetProjectionMatrix();

    // ---- パイプライン共通設定 ----
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // ---- ライトバッファの更新 ----
    LightBuffer lb;
    lb.lightDirection = RENDERER_LIGHT_DIR;
    lb.emissiveIntensity = RENDERER_EMISSIVE_INTENSITY;
    lb.lightColor = RENDERER_LIGHT_COLOR;
    lb.specularIntensity = RENDERER_SPECULAR_INTENSITY;
    lb.cameraPosition = camera.GetPosition();
    lb.shininess = RENDERER_SHININESS;
    m_deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, nullptr, &lb, 0, 0);
    m_deviceContext->PSSetConstantBuffers(1, 1, m_lightBuffer.GetAddressOf());

    // ---- 床の描画 ----
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ConstantBuffer cb;
    XMMATRIX floorWorld = XMMatrixScaling(floorScale, 1.f, floorScale);
    cb.wvp = XMMatrixTranspose(floorWorld * m_view * m_projection); // 行→列優先変換
    cb.world = XMMatrixTranspose(floorWorld);
    cb.objectColor = m_floorColor;
    m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    m_deviceContext->IASetVertexBuffers(0, 1, m_floorVertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_floorIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_deviceContext->DrawIndexed(6, 0, 0);
}

void Renderer::DrawCube(const XMMATRIX &worldMatrix, const XMFLOAT4 &color)
{
    ConstantBuffer cb;
    // HLSL の cbuffer は列優先(column-major)だが DirectXMath は行優先(row-major)。
    // GPU へ送る前に転置して列優先に変換する。
    cb.wvp = XMMatrixTranspose(worldMatrix * m_view * m_projection);
    cb.world = XMMatrixTranspose(worldMatrix);
    cb.objectColor = color;

    m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_deviceContext->DrawIndexed(36, 0, 0);
};

bool Renderer::InitSpriteBatch()
{
    m_spriteBatch = std::make_unique<SpriteBatch>(m_deviceContext.Get());
    m_states = std::make_unique<CommonStates>(m_device.Get());
    try
    {
        m_spriteFont = std::make_unique<SpriteFont>(m_device.Get(), UI_FONT_PATH);
    }
    catch (...)
    {
        MessageBox(nullptr, L"フォントファイルの読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    // SpriteBatch::Draw は必ずテクスチャを要求する。
    // 1×1 の白テクスチャを用意し RGBA 引数で色を指定することで
    // テクスチャなしのベタ塗り矩形として機能させる。
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    uint32_t white = 0xFFFFFFFF;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &white;
    initData.SysMemPitch = sizeof(uint32_t);

    ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, tex.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_device->CreateShaderResourceView(tex.Get(), nullptr, m_whiteTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    // タイトル背景画像の読み込み
    ComPtr<ID3D11Resource> titleBgResource;
    hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),
        UI_TITLE_BG_IMAGE_PATH,
        titleBgResource.GetAddressOf(),
        m_titleBgTexture.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"タイトル背景画像の読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    // タイトルロゴ画像の読み込み
    ComPtr<ID3D11Resource> titleResource;
    hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),
        UI_TITLE_IMAGE_PATH,
        titleResource.GetAddressOf(),
        m_titleTexture.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"タイトル画像の読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    ComPtr<ID3D11Resource> howToPlayResource;
    hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),
        UI_HOW_TO_PLAY_IMAGE_PATH,
        howToPlayResource.GetAddressOf(),
        m_howToPlayTexture.GetAddressOf());

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"タイトル画像の読み込みに失敗", L"エラー", MB_OK);
        return false;
    }

    // 画像サイズを取得（中央揃え計算用）
    ComPtr<ID3D11Texture2D> titleTex2D;
    titleResource.As(&titleTex2D);
    D3D11_TEXTURE2D_DESC titleTexDesc = {};
    titleTex2D->GetDesc(&titleTexDesc);
    m_titleTexWidth = titleTexDesc.Width;
    m_titleTexHeight = titleTexDesc.Height;

    return true;
}

void Renderer::DrawHP(int hp)
{
    wchar_t hpBar[PLAYER_HP + 1];
    for (int i = 0; i < PLAYER_HP; i++)
        hpBar[i] = (i < hp) ? L'#' : L'-';
    hpBar[PLAYER_HP] = L'\0';

    wchar_t buf[32];
    swprintf_s(buf, L"HP [%s]", hpBar);

    const RECT hudBar = {0, 0, WINDOW_WIDTH, static_cast<LONG>(UI_HUD_BAR_HEIGHT)};

    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
    m_spriteBatch->Draw(m_whiteTexture.Get(), hudBar, XMLoadFloat4(&UI_HUD_BAR_COLOR));
    m_spriteFont->DrawString(m_spriteBatch.get(), buf,
                             UI_HUD_HP_POS,
                             Colors::White, 0.0f,
                             XMFLOAT2(0, 0), UI_HUD_TEXT_SCALE);
    m_spriteBatch->End();
}

void Renderer::DrawCheckPoint(int getNum, size_t wholeNum)
{
    wchar_t buf[32];
    swprintf_s(buf, L"CP : %d/%zu", getNum, wholeNum);

    m_spriteBatch->Begin();
    m_spriteFont->DrawString(m_spriteBatch.get(), buf,
                             UI_HUD_CP_POS,
                             Colors::White, 0.0f,
                             XMFLOAT2(0, 0), UI_HUD_TEXT_SCALE);
    m_spriteBatch->End();
}

void Renderer::DrawTime(float time)
{
    int minute = (int)time / 60;
    int second = (int)time % 60;

    wchar_t buf[32];
    swprintf_s(buf, L"TIME : %02d:%02d", minute, second);

    m_spriteBatch->Begin();
    m_spriteFont->DrawString(m_spriteBatch.get(), buf,
                             UI_HUD_TIME_POS,
                             Colors::White, 0.0f,
                             XMFLOAT2(0, 0), UI_HUD_TEXT_SCALE);
    m_spriteBatch->End();
}

void Renderer::Clear(float r, float g, float b)
{
    float clearColor[4] = {r, g, b, 1.0f};
    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::DrawTitle()
{
    const wchar_t *startText = L"START";
    const wchar_t *exitText = L"EXIT";

    XMVECTOR startTextSize = m_spriteFont->MeasureString(startText);
    XMVECTOR exitTextSize = m_spriteFont->MeasureString(exitText);

    auto centeredTextPos = [&](XMVECTOR textSize, const RECT &rect) -> XMFLOAT2
    {
        float w = XMVectorGetX(textSize) * UI_BUTTON_TEXT_SCALE;
        float h = XMVectorGetY(textSize) * UI_BUTTON_TEXT_SCALE;
        return XMFLOAT2(
            rect.left + (rect.right - rect.left - w) * 0.5f,
            rect.top + (rect.bottom - rect.top - h) * 0.5f);
    };

    XMFLOAT2 startTextPos = centeredTextPos(startTextSize, TITLE_START_BUTTON_RECT);
    XMFLOAT2 exitTextPos = centeredTextPos(exitTextSize, TITLE_EXIT_BUTTON_RECT);

    // ボタン上のスペース（Y=0〜340）にロゴを収める
    const float logoAreaH = static_cast<float>(TITLE_START_BUTTON_RECT.top);
    float scale = (m_titleTexHeight > 0)
                      ? std::min(logoAreaH * 0.8f / m_titleTexHeight,
                                 static_cast<float>(WINDOW_WIDTH) * 0.75f / m_titleTexWidth)
                      : 1.0f;
    float logoW = m_titleTexWidth * scale;
    float logoH = m_titleTexHeight * scale;
    XMFLOAT2 logoPos(
        (WINDOW_WIDTH - logoW) * 0.5f,
        (logoAreaH - logoH) * 0.5f);

    const RECT fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_titleBgTexture.Get(), fullscreen);
    m_spriteBatch->Draw(m_titleTexture.Get(), logoPos, nullptr,
                        Colors::White, 0.0f, XMFLOAT2(0, 0), scale);
    m_spriteBatch->Draw(m_whiteTexture.Get(), TITLE_START_BUTTON_RECT,
                        XMLoadFloat4(&UI_TITLE_START_BTN_COLOR));
    m_spriteFont->DrawString(m_spriteBatch.get(), startText, startTextPos,
                             Colors::White, 0.0f, XMFLOAT2(0, 0), UI_BUTTON_TEXT_SCALE);
    m_spriteBatch->Draw(m_whiteTexture.Get(), TITLE_EXIT_BUTTON_RECT,
                        XMLoadFloat4(&UI_TITLE_EXIT_BTN_COLOR));
    m_spriteFont->DrawString(m_spriteBatch.get(), exitText, exitTextPos,
                             Colors::White, 0.0f, XMFLOAT2(0, 0), UI_BUTTON_TEXT_SCALE);
    m_spriteBatch->End();
}

void Renderer::DrawResultScreen(const wchar_t *title, char rankChar, const XMFLOAT4 &titleColor, const XMFLOAT4 &btnColor)
{
    const wchar_t *btnText = L"TITLE";
    wchar_t rankStr[32];
    swprintf_s(rankStr, L"SYNCHRO RANK : %c", rankChar);

    XMVECTOR titleSize = m_spriteFont->MeasureString(title);
    XMVECTOR btnTextSize = m_spriteFont->MeasureString(btnText);
    XMVECTOR rankSize = m_spriteFont->MeasureString(rankStr);

    XMFLOAT2 titlePos(
        (WINDOW_WIDTH - XMVectorGetX(titleSize)) * 0.5f,
        WINDOW_HEIGHT * 0.5f - XMVectorGetY(titleSize) * 0.5f + UI_RESULT_TITLE_OFFSET_Y);

    XMFLOAT2 rankPos(
        (WINDOW_WIDTH - XMVectorGetX(rankSize)) * 0.5f,
        WINDOW_HEIGHT * 0.5f - XMVectorGetY(rankSize) * 0.5f + UI_RESULT_RANK_OFFSET_Y);

    float textScaledW = XMVectorGetX(btnTextSize) * UI_BUTTON_TEXT_SCALE;
    float textScaledH = XMVectorGetY(btnTextSize) * UI_BUTTON_TEXT_SCALE;
    XMFLOAT2 btnTextPos(
        GAME_EXIT_BUTTON_RECT.left + (GAME_EXIT_BUTTON_RECT.right - GAME_EXIT_BUTTON_RECT.left - textScaledW) * 0.5f,
        GAME_EXIT_BUTTON_RECT.top + (GAME_EXIT_BUTTON_RECT.bottom - GAME_EXIT_BUTTON_RECT.top - textScaledH) * 0.5f);

    const RECT fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
    m_spriteBatch->Draw(m_whiteTexture.Get(), fullscreen, XMLoadFloat4(&UI_RESULT_OVERLAY_COLOR));
    m_spriteFont->DrawString(m_spriteBatch.get(), title, titlePos, XMLoadFloat4(&titleColor));
    m_spriteFont->DrawString(m_spriteBatch.get(), rankStr, rankPos, XMLoadFloat4(&UI_RESULT_RANK_COLOR));
    m_spriteBatch->Draw(m_whiteTexture.Get(), GAME_EXIT_BUTTON_RECT, XMLoadFloat4(&btnColor));
    m_spriteFont->DrawString(m_spriteBatch.get(), btnText, btnTextPos,
                             Colors::White, 0.0f, XMFLOAT2(0, 0), UI_BUTTON_TEXT_SCALE);
    m_spriteBatch->End();
}

void Renderer::DrawGameOver(char rankChar)
{
    DrawResultScreen(L"GAME OVER", rankChar, UI_GAME_OVER_TITLE_COLOR, UI_GAME_OVER_BTN_COLOR);
}

void Renderer::DrawGameClear(char rankChar)
{
    DrawResultScreen(L"GAME CLEAR", rankChar, UI_GAME_CLEAR_TITLE_COLOR, UI_GAME_CLEAR_BTN_COLOR);
}

void Renderer::DrawHowToPlay()
{
    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_howToPlayTexture.Get(), HOW_TO_PLAY_DRAW_RECT);
    m_spriteBatch->End();
}

void Renderer::Present()
{
    m_swapChain->Present(1, 0);
}