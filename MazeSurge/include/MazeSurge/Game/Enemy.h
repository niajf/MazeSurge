#pragma once
#include "MazeSurge/Core/Core.h"

struct Enemy
{
    XMFLOAT4 color;    // 描画色
    XMFLOAT3 position; // ワールド空間における中心座標
    BBOX bbox;         // 衝突判定領域（AABB）
    bool active;       // プール内でアクティブかどうか（false なら使用されていない）
    float scale;       // キューブの辺の長さ
    float speed;       // 移動速度（単位/秒）

    // デフォルト値で各フィールドを初期化するコンストラクタ
    Enemy();
};