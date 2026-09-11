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
    // MinDepth=0.0, MaxDepth=1.0 は D3D11 の標準深度範囲。
    // 変更すると深度テスト結果が狂うため、明示的にデフォルト値をセットしている。
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
    // リフレッシュレートを 60/1 Hz に固定。
    // Windowed=TRUE の場合、実際のリフレッシュはモニター依存になるが
    // DISCARD スワップエフェクトと組み合わせると垂直同期待ち時間が短くなる。
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 8bpc RGBA（ガンマ補正なし）
    scd.SampleDesc.Count = 1;                           // マルチサンプリング無効（AA なし）
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    // BufferCount=1 はフロントバッファのみ。
    // DXGI_SWAP_EFFECT_DISCARD では Present 後の内容保証が不要なため 1 枚で十分。
    scd.BufferCount = 1;
    scd.OutputWindow = hwnd;
    scd.Windowed = TRUE;
    // DISCARD: Present 後にバックバッファの内容を破棄する。
    // 毎フレーム Clear するので内容保持は不要であり、最もオーバーヘッドが少ない。
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    // デバッグレイヤーを有効にすると D3D11 の API 呼び出しエラーが
    // OutputDebugString に詳細メッセージとして出力される。
    // リリースビルドでは有効化すると速度低下するため除外する。
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // 優先度順にフィーチャーレベルを列挙し、最上位で利用可能なものが選択される。
    // D3D_FEATURE_LEVEL_11_0 が取得できなくてもゲームは動作するが、
    // 11_0 未満では一部のシェーダーモデル(SM 5.0)命令が使えないことがある。
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL featureLevelOut; // 実際に採用されたフィーチャーレベル（現在は未参照）

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                  // アダプター指定なし → デフォルト GPU
        D3D_DRIVER_TYPE_HARDWARE, // HW アクセラレーション（WARP/REF は不使用）
        nullptr,
        createDeviceFlags,
        featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
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
    // ---- レンダーターゲットビュー ----
    // GetBuffer(0) でスワップチェインのバックバッファ（インデックス 0）を取得する。
    // BufferCount=1 なので使えるバックバッファはこれ 1 枚のみ。
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                        reinterpret_cast<void **>(backBuffer.GetAddressOf()));
    if (FAILED(hr))
        return false;

    // nullptr を渡すと、バックバッファ自身のフォーマット/ミップ設定をそのまま使う。
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                          m_renderTargetView.GetAddressOf());
    if (FAILED(hr))
        return false;

    // ---- 深度ステンシルバッファ ----
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = WINDOW_WIDTH;
    depthDesc.Height = WINDOW_HEIGHT;
    depthDesc.MipLevels = 1; // 深度バッファにミップマップは不要
    depthDesc.ArraySize = 1;
    // D24_UNORM_S8_UINT: 深度 24 bit（0.0〜1.0 正規化）＋ステンシル 8 bit。
    // ステンシルは現在未使用だが、このフォーマットが最も広くサポートされている。
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1; // RTV と一致させる（マルチサンプルなし）
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthTexture;
    hr = m_device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    // nullptr を渡すとテクスチャ全体をビューとして使用する（サブリソース指定なし）。
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
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS; // 暗黙の型変換などを警告/エラー扱いにする
#ifdef _DEBUG
    // デバッグ情報を埋め込み、最適化を無効化することで PIX / RenderDoc でのデバッグを容易にする。
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filePath,
        nullptr,                           // マクロ定義なし
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // HLSL ファイル内の #include を解決できるようにする
        entryPoint,
        profile,
        compileFlags,
        0,
        blob.GetAddressOf(),
        errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        // errorBlob にコンパイルエラーの詳細文字列が格納されている。
        // VS の出力ウィンドウで確認できるよう OutputDebugStringA で送出する。
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
    // ---- 頂点シェーダー (vs_5_0 = SM 5.0) ----
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

    // ---- ピクセルシェーダー (ps_5_0 = SM 5.0) ----
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

    // 入力レイアウトの作成には頂点シェーダーのバイトコードが必要。
    // PS コンパイル後に行うのはその制約を満たすため。
    if (!CreateInputLayout(vsBlob))
        return false;

    return true;
}

// ============================================================
// 入力レイアウトの作成
// ============================================================
bool Renderer::CreateInputLayout(ComPtr<ID3DBlob> &vsBlob)
{
    // 各要素の AlignedByteOffset は Vertex 構造体のメンバ順に合わせている。
    // POSITION: オフセット 0  (float3 = 12 bytes)
    // NORMAL  : オフセット 12 (float3 = 12 bytes)
    // TEXCOORD: オフセット 24 (float2 =  8 bytes)
    // 合計ストライド = 32 bytes。Vertex 構造体のサイズと一致する必要がある。
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // 入力レイアウトは VS バイトコードと照合して整合性を検証する。
    // VS を再コンパイルした場合はレイアウトも再作成しなければならない。
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
    // ---- WVP（ワールド・ビュー・プロジェクション）用定数バッファ ----
    // D3D11_USAGE_DEFAULT: CPU 書き込みは UpdateSubresource 経由で行う。
    // ByteWidth は 16 バイトアライメントが必要（HLSL cbuffer の制約）。
    // sizeof(ConstantBuffer) が 16 の倍数であることをヘッダー側で保証すること。
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr = m_device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    // ---- ライトパラメータ用定数バッファ ----
    // 方向光源・スペキュラー・エミッシブなど光源情報を GPU に送るためのバッファ。
    // VS スロット 0 は WVP バッファが占有しているため、PS スロット 1 に割り当てる。
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
    // 各面は独立した 4 頂点を持つ（法線が面ごとに異なるため共有できない）。
    // 頂点レイアウト: {位置, 法線, UV}
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

    // 各面 2 トライアングル = 6 インデックス、6 面で合計 36 インデックス。
    // パターン: {n, n+1, n+2, n, n+2, n+3} (左回り = 反時計回り CCW)
    UINT cubeIndices[] =
        {
            0,
            1,
            2,
            0,
            2,
            3, // 前面
            4,
            5,
            6,
            4,
            6,
            7, // 背面
            8,
            9,
            10,
            8,
            10,
            11, // 上面
            12,
            13,
            14,
            12,
            14,
            15, // 底面
            16,
            17,
            18,
            16,
            18,
            19, // 左面
            20,
            21,
            22,
            20,
            22,
            23, // 右面
    };

    // ---- 床頂点データ ----
    // y = -0.5 に配置し、キューブ（高さ 1.0 = -0.5〜+0.5）の底と揃える。
    // UV を (0,0)〜(10,10) にすることでサンプラーのラップ設定によりテクスチャが繰り返される。
    Vertex floorVertices[] =
        {
            {XMFLOAT3(-1.f, -0.5f, -1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0)},
            {XMFLOAT3(1.f, -0.5f, -1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(10, 0)},
            {XMFLOAT3(1.f, -0.5f, 1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(10, 10)},
            {XMFLOAT3(-1.f, -0.5f, 1.f), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 10)},
    };

    // 床は 2 トライアングル = 6 インデックス。
    // {0,2,1, 0,3,2} は上から見て時計回り（CW）になるよう頂点順を調整している。
    UINT floorIndices[] = {0, 2, 1, 0, 3, 2};

    // ---- バッファ作成ヘルパー ----
    // GPU 側で内容が変わらない（USAGE_DEFAULT）頂点・インデックスバッファを一括作成する。
    auto createBuffer = [&](auto *data, UINT byteWidth, UINT bindFlags, ComPtr<ID3D11Buffer> &buffer)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = byteWidth;
        desc.BindFlags = bindFlags;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data; // 初期データの CPU 側ポインタ

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
    // SpriteBatch::End() は深度・ブレンド・ラスタライザステートを内部で変更する。
    // nullptr を渡すと D3D11 デフォルト設定に戻り、次の 3D パスが正しく描画される。
    // この呼び出しを省略すると SpriteBatch が設定したステートが 3D 描画に引き継がれ、
    // 深度テスト失敗やブレンド異常が発生する。
    m_deviceContext->OMSetDepthStencilState(nullptr, 0);
    m_deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    m_deviceContext->RSSetState(nullptr);

    // ---- ビュー・プロジェクション行列をメンバにキャッシュ ----
    // DrawCube は Render の後に呼ばれるため、ここでキャッシュしておく必要がある。
    m_view = camera.GetViewMatrix();
    m_projection = camera.GetProjectionMatrix();

    // ---- パイプライン共通設定 ----
    // 入力レイアウト・トポロジー・シェーダーは全描画で共通。
    // 頂点ごとのワールド変換は定数バッファで切り替える。
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    // スロット 0: WVP 定数バッファ。VS / PS 両方にバインドする。
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // ---- ライトバッファの更新 ----
    // ライトパラメータはフレームごとに変化しないが、カメラ位置はスペキュラー計算に使う。
    // UpdateSubresource で GPU 側バッファへ転送する。
    LightBuffer lb;
    lb.lightDirection = RENDERER_LIGHT_DIR;
    lb.emissiveIntensity = RENDERER_EMISSIVE_INTENSITY;
    lb.lightColor = RENDERER_LIGHT_COLOR;
    lb.specularIntensity = RENDERER_SPECULAR_INTENSITY;
    lb.cameraPosition = camera.GetPosition(); // スペキュラー計算用の視点位置
    lb.shininess = RENDERER_SHININESS;
    m_deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, nullptr, &lb, 0, 0);
    m_deviceContext->PSSetConstantBuffers(1, 1, m_lightBuffer.GetAddressOf()); // スロット 1 に配置

    // ---- 床の描画 ----
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ConstantBuffer cb;
    // floorScale でワールド座標系の床サイズを動的に変更できる。
    // Y・Z 方向はスケール 1.0 固定とし、床は水平面のみ拡縮する。
    XMMATRIX floorWorld = XMMatrixScaling(floorScale, 1.f, floorScale);
    // HLSL の cbuffer は列優先(column-major)だが DirectXMath は行優先(row-major)。
    // XMMatrixTranspose で転置してから GPU へ送ることで、シェーダー内の mul() が正しく機能する。
    cb.wvp = XMMatrixTranspose(floorWorld * m_view * m_projection);
    cb.world = XMMatrixTranspose(floorWorld);
    cb.objectColor = m_floorColor;
    m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    m_deviceContext->IASetVertexBuffers(0, 1, m_floorVertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_floorIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_deviceContext->DrawIndexed(6, 0, 0); // 床は 6 インデックス = 2 トライアングル
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
    m_deviceContext->DrawIndexed(36, 0, 0); // キューブは 36 インデックス = 6 面 × 2 トライアングル × 3
};

bool Renderer::InitSpriteBatch()
{
    // SpriteBatch: DirectXTK が提供する 2D スプライト描画クラス。
    // CommonStates: よく使うブレンド・サンプラー・ラスタライザステートのファクトリ。
    m_spriteBatch = std::make_unique<SpriteBatch>(m_deviceContext.Get());
    m_states = std::make_unique<CommonStates>(m_device.Get());
    try
    {
        // フォントファイルが見つからない場合は例外を投げる。
        // catch で MessageBox を出しデバッグを助ける。
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

    // 0xFFFFFFFF = RGBA (255, 255, 255, 255) の不透明白ピクセル。
    // SpriteBatch の tint カラーと乗算されるため、白であれば色がそのまま反映される。
    uint32_t white = 0xFFFFFFFF;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &white;
    initData.SysMemPitch = sizeof(uint32_t); // 1 ピクセル幅 = 4 bytes

    ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, tex.GetAddressOf());
    if (FAILED(hr))
        return false;

    // シェーダーリソースビューを作成してシェーダーからテクスチャを参照可能にする。
    hr = m_device->CreateShaderResourceView(tex.Get(), nullptr, m_whiteTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    // ---- タイトル背景画像の読み込み ----
    // CreateWICTextureFromFile は WIC 対応の画像形式（PNG/JPEG 等）を直接読み込める。
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

    // ---- タイトルロゴ画像の読み込み ----
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

    // ロゴのアスペクト比を保ったままスケーリングするために元サイズを取得する。
    // titleResource を ID3D11Texture2D にキャストして GetDesc() でサイズを読み出す。
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
    // '#' で残り HP、'-' で消費済み HP を表すバーを文字列で生成する。
    wchar_t hpBar[PLAYER_HP + 1];
    for (int i = 0; i < PLAYER_HP; i++)
        hpBar[i] = (i < hp) ? L'#' : L'-';
    hpBar[PLAYER_HP] = L'\0';

    wchar_t buf[32];
    swprintf_s(buf, L"HP [%s]", hpBar);

    // HUD バー: 画面上部に固定高の矩形を描画する。
    const RECT hudBar = {0, 0, WINDOW_WIDTH, static_cast<LONG>(UI_HUD_BAR_HEIGHT)};

    // NonPremultiplied: テクスチャのアルファが事前乗算されていない場合に使うブレンドステート。
    // SpriteFont の文字テクスチャは事前乗算なしのアルファを持つため、このモードを使う。
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
    // "CP : 取得数/全数" 形式で表示する。
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
    // float 秒を分・秒に分解して "TIME : MM:SS" 形式で表示する。
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
    // OMSetRenderTargets で RTV と DSV を同時に設定する。
    // 毎フレーム呼び出すことで、前フレームの SpriteBatch がバインドを解除していても復元できる。
    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    // D3D11_CLEAR_DEPTH: 深度バッファを 1.0（最遠値）でクリアする。
    // ステンシルは使用しないため D3D11_CLEAR_STENCIL は指定しない。
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::DrawTitle()
{
    const wchar_t *startText = L"START";
    const wchar_t *exitText = L"EXIT";

    // MeasureString でテキストの描画サイズ（ピクセル）を取得する。
    // ボタン矩形内での中央揃え座標の計算に使う。
    XMVECTOR startTextSize = m_spriteFont->MeasureString(startText);
    XMVECTOR exitTextSize = m_spriteFont->MeasureString(exitText);

    // ボタン矩形内でテキストを水平・垂直中央揃えにする座標を返すラムダ。
    // スケールを考慮した実際のテキスト幅・高さから余白を求めて left/top に加算する。
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

    // ロゴをボタン上方の空きスペース（Y=0〜TITLE_START_BUTTON_RECT.top）に収める。
    // 幅と高さの両方について収まるスケールを求め、小さい方（アスペクト比保持）を採用する。
    const float logoAreaH = static_cast<float>(TITLE_START_BUTTON_RECT.top);
    float scale = (m_titleTexHeight > 0)
                      ? std::min(logoAreaH * 0.8f / m_titleTexHeight,
                                 static_cast<float>(WINDOW_WIDTH) * 0.75f / m_titleTexWidth)
                      : 1.0f;
    float logoW = m_titleTexWidth * scale;
    float logoH = m_titleTexHeight * scale;
    // ロゴをロゴエリア内で水平・垂直中央に配置する。
    XMFLOAT2 logoPos(
        (WINDOW_WIDTH - logoW) * 0.5f,
        (logoAreaH - logoH) * 0.5f);

    const RECT fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_titleBgTexture.Get(), fullscreen); // 背景
    m_spriteBatch->Draw(m_titleTexture.Get(), logoPos, nullptr,
                        Colors::White, 0.0f, XMFLOAT2(0, 0), scale); // ロゴ
    m_spriteBatch->Draw(m_whiteTexture.Get(), TITLE_START_BUTTON_RECT,
                        XMLoadFloat4(&UI_TITLE_START_BTN_COLOR)); // START ボタン背景
    m_spriteFont->DrawString(m_spriteBatch.get(), startText, startTextPos,
                             Colors::White, 0.0f, XMFLOAT2(0, 0), UI_BUTTON_TEXT_SCALE);
    m_spriteBatch->Draw(m_whiteTexture.Get(), TITLE_EXIT_BUTTON_RECT,
                        XMLoadFloat4(&UI_TITLE_EXIT_BTN_COLOR)); // EXIT ボタン背景
    m_spriteFont->DrawString(m_spriteBatch.get(), exitText, exitTextPos,
                             Colors::White, 0.0f, XMFLOAT2(0, 0), UI_BUTTON_TEXT_SCALE);
    m_spriteBatch->End();
}

// DrawGameOver / DrawGameClear は共通の結果画面レイアウトを共有するため
// DrawResultScreen に処理を委譲し、タイトル文字列・色だけを差し替える。
void Renderer::DrawResultScreen(const wchar_t *title, char rankChar, const XMFLOAT4 &titleColor, const XMFLOAT4 &btnColor)
{
    const wchar_t *btnText = L"TITLE";
    wchar_t mediumStr[32];
    wchar_t rankStr[8];

    swprintf_s(mediumStr, L"SYNCHRO RANK");
    swprintf_s(rankStr, L"%c", rankChar);

    // 各テキストの描画サイズを測定して中央揃え座標を計算する。
    XMVECTOR titleSize = m_spriteFont->MeasureString(title);
    XMVECTOR btnTextSize = m_spriteFont->MeasureString(btnText);
    XMVECTOR mediumSize = m_spriteFont->MeasureString(mediumStr);
    XMVECTOR rankSize = m_spriteFont->MeasureString(rankStr);

    // タイトル文字を画面垂直中央から UI_RESULT_TITLE_OFFSET_Y だけずらした位置に配置する。
    XMFLOAT2 titlePos(
        (WINDOW_WIDTH - XMVectorGetX(titleSize)) * 0.5f,
        WINDOW_HEIGHT * 0.5f - XMVectorGetY(titleSize) * 0.5f + UI_RESULT_TITLE_OFFSET_Y);

    // ランク文字も同様に垂直中央からずらして配置する。
    XMFLOAT2 mediumPos(
        (WINDOW_WIDTH - XMVectorGetX(mediumSize)) * 0.5f,
        WINDOW_HEIGHT * 0.5f - XMVectorGetY(mediumSize) * 0.5f + UI_RESULT_RANK_OFFSET_Y);

    XMFLOAT2 rankPos(
        (WINDOW_WIDTH - XMVectorGetX(rankSize)) * 0.5f,
        WINDOW_HEIGHT * 0.5f - XMVectorGetY(rankSize) * 0.5f + UI_RESULT_RANK_OFFSET_Y + 80.f);

    // ボタン矩形内でテキストを中央揃えにする座標を計算する。
    float textScaledW = XMVectorGetX(btnTextSize) * UI_BUTTON_TEXT_SCALE;
    float textScaledH = XMVectorGetY(btnTextSize) * UI_BUTTON_TEXT_SCALE;
    XMFLOAT2 btnTextPos(
        GAME_EXIT_BUTTON_RECT.left + (GAME_EXIT_BUTTON_RECT.right - GAME_EXIT_BUTTON_RECT.left - textScaledW) * 0.5f,
        GAME_EXIT_BUTTON_RECT.top + (GAME_EXIT_BUTTON_RECT.bottom - GAME_EXIT_BUTTON_RECT.top - textScaledH) * 0.5f);

    const RECT fullscreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

    // NonPremultiplied: 半透明オーバーレイを正しくブレンドするために必要。
    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

    // オーバーレイ
    m_spriteBatch->Draw(m_whiteTexture.Get(), fullscreen, XMLoadFloat4(&UI_RESULT_OVERLAY_COLOR)); // 暗転オーバーレイ

    // テキスト
    m_spriteFont->DrawString(m_spriteBatch.get(), title, titlePos, XMLoadFloat4(&titleColor));
    m_spriteFont->DrawString(m_spriteBatch.get(), mediumStr, mediumPos, XMLoadFloat4(&UI_RESULT_RANK_COLOR));
    m_spriteFont->DrawString(m_spriteBatch.get(), rankStr, rankPos, XMLoadFloat4(&UI_RESULT_RANK_COLOR));

    // ボタン
    m_spriteBatch->Draw(m_whiteTexture.Get(), GAME_EXIT_BUTTON_RECT, XMLoadFloat4(&btnColor)); // ボタン背景
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
    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
    // HOW_TO_PLAY_DRAW_RECT で指定した矩形にテクスチャをストレッチ描画する。
    m_spriteBatch->Draw(m_howToPlayTexture.Get(), HOW_TO_PLAY_DRAW_RECT);
    m_spriteBatch->End();
}

void Renderer::Present()
{
    // SyncInterval=1: 垂直同期を有効にし、ティアリングを防ぐ。
    // 0 にするとモニターのリフレッシュレートを超えたフレームレートが出るが
    // ティアリングが発生する可能性がある。
    m_swapChain->Present(1, 0);
}
