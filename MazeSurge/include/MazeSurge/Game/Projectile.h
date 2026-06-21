#pragma once
#include "MazeSurge/Core/Core.h"

struct Projectile
{
    XMFLOAT4 color;     // 描画色
    XMFLOAT3 position;  // ワールド空間における中心座標
    XMFLOAT3 direction; // 正規化された移動方向ベクトル
    BBOX bbox;          // 衝突判定領域（AABB）
    bool active;        // プール内でアクティブかどうか（false なら使用されていない）
    float scale;        // キューブの辺の長さ
    float speed;        // 移動速度（単位/秒）
    float lifetime;     // 生存してからの経過時間
    float maxLifetime;  // 寿命の上限（超えると非アクティブになる）

    // デフォルト値で各フィールドを初期化するコンストラクタ
    Projectile();
};