#pragma once
#include "MazeSurge/Core/Common.h"

// ============================================================
// 頂点構造体
// ============================================================
// GPU側のHLSL構造体（VSInput）と完全に対応する。
// メモリレイアウト:
//   POSITION  — オフセット 0  (12バイト)
//   NORMAL    — オフセット 12 (12バイト)
//   TEXCOORD  — オフセット 24 (8バイト)
//   合計: 32バイト/頂点
struct Vertex
{
    XMFLOAT3 position; // 位置 (x, y, z)
    XMFLOAT3 normal;   // 法線ベクトル
    XMFLOAT2 texCoord; // UV座標
};

// ============================================================
// 定数バッファ構造体（register(b0)に対応）
// ============================================================
// alignas(16): D3D11 の定数バッファは 16 バイト境界が必須。
// XMMATRIX(64B) + XMMATRIX(64B) + XMFLOAT4(16B) = 144B で自然に満たす。
// GPU にアップロードする前に XMMatrixTranspose で転置すること：
//   DirectXMath は行優先(row-major)、HLSL の cbuffer は列優先(column-major)。
struct alignas(16) ConstantBuffer
{
    XMMATRIX wvp;   // ワールド × ビュー × プロジェクション行列
    XMMATRIX world; // ワールド行列（ライティング用）
    XMFLOAT4 objectColor;
};

// ============================================================
// ライト用定数バッファ構造体（register(b1)に対応）
// ============================================================
// HLSL の float4 パッキング規則: float3 の後に float を置いて
// 各フィールドが 16B 境界をまたがないようにレイアウトしている。
struct alignas(16) LightBuffer
{
    XMFLOAT3 lightDirection;
    float emissiveIntensity; // float3+float = 16B
    XMFLOAT3 lightColor;
    float specularIntensity; // float3+float = 16B
    XMFLOAT3 cameraPosition;
    float shininess; // float3+float = 16B
};

// ============================================================
// BoundinBox構造体(AABB衝突判定に使用)
// ============================================================
// XZ 平面のみを対象とする 2D AABB。
// ゲーム内の全オブジェクトは同一の地面高度に存在するため Y 軸は不要。
struct BBOX
{
    float minX;
    float maxX;
    float minZ;
    float maxZ;

    // 中心座標 pos と辺の長さ scale から AABB の各端を計算して設定する
    void setBBOX(const XMFLOAT3 &pos, float scale)
    {
        minX = pos.x - scale * 0.5f;
        maxX = pos.x + scale * 0.5f;
        minZ = pos.z - scale * 0.5f;
        maxZ = pos.z + scale * 0.5f;
    }
};

// ============================================================
// 入力構造体(mainから各シーンに入力を伝えるために使用)
// ============================================================
struct InputState
{
    bool keyW = false;           // W キーが押されているか
    bool keyA = false;           // A キーが押されているか
    bool keyS = false;           // S キーが押されているか
    bool keyD = false;           // D キーが押されているか
    bool lMousePrevDown = false; // 前フレームの左クリック状態（クリック検出に使用）
    bool lMouseDown = false;     // 現フレームの左クリック状態
    int mouseX = 0;              // 現フレームのマウスX座標（スクリーン座標）
    int mouseY = 0;              // 現フレームのマウスY座標（スクリーン座標）
};