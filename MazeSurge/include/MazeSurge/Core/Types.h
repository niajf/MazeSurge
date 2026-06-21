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
struct alignas(16) ConstantBuffer
{
    XMMATRIX wvp;   // ワールド × ビュー × プロジェクション行列
    XMMATRIX world; // ワールド行列（ライティング用）
    XMFLOAT4 objectColor;
};

// ============================================================
// ライト用定数バッファ構造体（register(b1)に対応）
// ============================================================
struct alignas(16) LightBuffer
{
    XMFLOAT3 lightDirection;
    float emissiveIntensity;
    XMFLOAT3 lightColor;
    float specularIntensity;
    XMFLOAT3 cameraPosition;
    float shininess;
};

// ============================================================
// BoundinBox構造体(AABB衝突判定に使用)
// ============================================================
struct BBOX
{
    float minX;
    float maxX;
    float minZ;
    float maxZ;

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
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool lMouseDown = false;
    int mouseX = 0;
    int mouseY = 0;
};