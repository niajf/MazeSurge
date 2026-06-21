#pragma once
#include "MazeSurge/Core/Common.h"

// ============================================================
// 頂点構造体
// ============================================================
// GPU 側の HLSL 構造体（VSInput）と完全に対応する。
// メモリレイアウト:
//   POSITION  — オフセット 0  (12 バイト)
//   NORMAL    — オフセット 12 (12 バイト)
//   TEXCOORD  — オフセット 24 ( 8 バイト)
//   合計: 32 バイト/頂点
// CreateInputLayout に渡す D3D11_INPUT_ELEMENT_DESC のオフセット値と必ず一致させること。
struct Vertex
{
    XMFLOAT3 position; // 位置 (x, y, z)
    XMFLOAT3 normal;   // 法線ベクトル（ライティング計算に使用）
    XMFLOAT2 texCoord; // UV 座標（テクスチャマッピング用）
};

// ============================================================
// 定数バッファ構造体（register(b0) に対応）
// ============================================================
// alignas(16): D3D11 の定数バッファは 16 バイトアライメントが必須。
// XMMATRIX(64B) × 2 + XMFLOAT4(16B) = 144B で自然に満たしているが、
// 将来フィールドを追加する場合はパディングに注意すること。
// GPU にアップロードする前に XMMatrixTranspose で転置すること:
//   DirectXMath は行優先(row-major)、HLSL の cbuffer は列優先(column-major)。
struct alignas(16) ConstantBuffer
{
    XMMATRIX wvp;         // ワールド × ビュー × プロジェクション行列（頂点変換用）
    XMMATRIX world;       // ワールド行列（法線のワールド変換・ライティング用）
    XMFLOAT4 objectColor; // オブジェクトの基本色（ピクセルシェーダーで乗算）
};

// ============================================================
// ライト用定数バッファ構造体（register(b1) に対応）
// ============================================================
// HLSL の float4 パッキング規則: float3 の後に float を置いて
// 各フィールドが 16B 境界をまたがないようにレイアウトしている。
// 例: lightDirection(12B) + emissiveIntensity(4B) = 16B で境界内に収まる。
// 順序を変えると GPU 側のレイアウトとずれて値が化ける。
struct alignas(16) LightBuffer
{
    XMFLOAT3 lightDirection; // 光源の方向ベクトル（正規化推奨）
    float emissiveIntensity; // 環境光（アンビエント）強度 — float3 + float = 16B
    XMFLOAT3 lightColor;     // 光源の色 (RGB)
    float specularIntensity; // スペキュラー反射の強度 — float3 + float = 16B
    XMFLOAT3 cameraPosition; // 視点位置（スペキュラー計算の反射ベクトルに使用）
    float shininess;         // Phong モデルの光沢指数（大きいほど鋭い反射） — float3 + float = 16B
};

// ============================================================
// バウンディングボックス構造体（AABB 衝突判定に使用）
// ============================================================
// XZ 平面のみを対象とする 2D AABB。
// ゲーム内の全オブジェクトは同一の地面高度に存在するため Y 軸の判定は不要。
// minX/maxX/minZ/maxZ の 4 値でボックスを表現する。
struct BBOX
{
    float minX;
    float maxX;
    float minZ;
    float maxZ;

    // 中心座標 pos と正方形の辺長 scale から AABB の端を計算して設定する。
    // scale は均一スケール（正方形）を前提とする。非均一スケールには対応していない。
    void setBBOX(const XMFLOAT3 &pos, float scale)
    {
        minX = pos.x - scale * 0.5f;
        maxX = pos.x + scale * 0.5f;
        minZ = pos.z - scale * 0.5f;
        maxZ = pos.z + scale * 0.5f;
    }
};

// ============================================================
// 入力状態構造体（main.cpp から各シーンへ入力を伝えるために使用）
// ============================================================
// WndProc でメッセージを受け取り、グローバルインスタンス inputState を更新する。
// ゲームループで Update() に渡し、各システムが参照する。
struct InputState
{
    bool keyW = false; // W キー押下中（前進）
    bool keyA = false; // A キー押下中（左移動）
    bool keyS = false; // S キー押下中（後退）
    bool keyD = false; // D キー押下中（右移動）
    // lMousePrevDown と lMouseDown を比較することで「ボタンを離した瞬間」を検出できる。
    // !lMouseDown && lMousePrevDown == true の時がクリック完了（Press → Release）。
    bool lMousePrevDown = false; // 前フレームの左ボタン状態
    bool lMouseDown = false;     // 現フレームの左ボタン状態
    int mouseX = 0;              // マウスカーソルの X 座標（クライアント座標系）
    int mouseY = 0;              // マウスカーソルの Y 座標（クライアント座標系）
};
