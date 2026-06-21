#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Enemy — 敵の状態データ（値型・struct）
// ============================================================
// ロジックは EnemyManager が担当し、Enemy 自体はデータのみを持つ。
// struct にすることで全メンバがデフォルト public となり、
// EnemyManager から直接アクセスできる（カプセル化より利便性を優先）。
// オブジェクトプールパターン：EnemyManager が固定サイズの vector<Enemy> を保持し、
// active フラグで使用中かどうかを管理する。
struct Enemy
{
    XMFLOAT4 color;    // 描画色
    XMFLOAT3 position; // ワールド空間における中心座標（Y=0 固定）
    BBOX bbox;         // 衝突判定領域（AABB、XZ 平面のみ）
    // active=false の Enemy はプール内の空きスロット。
    // 描画・衝突判定・移動処理はいずれも active=true の場合のみ行う。
    bool active;
    float scale; // キューブの辺の長さ（均一スケール）
    float speed; // 移動速度（単位/秒）

    // コンストラクタでデフォルト値を設定する。
    // active=false に初期化して、SpawnEnemy() で明示的に有効化するまで使用されない。
    Enemy();
};
