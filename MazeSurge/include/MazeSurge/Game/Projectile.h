#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Projectile — 弾の状態データ（値型・struct）
// ============================================================
// ロジックは ProjectilePool が担当し、Projectile はデータのみを持つ。
// Enemy と同様に struct + active フラグによるオブジェクトプールパターン。
struct Projectile
{
    XMFLOAT4 color;    // 描画色
    XMFLOAT3 position; // ワールド空間における中心座標
    // direction は Get() 呼び出し時に正規化されたベクトルを設定する。
    // 正規化しておくことで Update() での速度計算が direction × speed になる。
    XMFLOAT3 direction; // 移動方向（正規化済み）
    BBOX bbox;          // 衝突判定領域（AABB、XZ 平面のみ）
    // active=false の Projectile はプール内の空きスロット。
    bool active;
    float scale; // キューブの辺の長さ（均一スケール）
    float speed; // 移動速度（単位/秒）
    // lifetime は発射時に 0 にリセットされ、毎フレーム deltaTime が加算される。
    float lifetime;    // 発射からの経過時間（秒）
    float maxLifetime; // 寿命の上限（超えると active=false になる）

    // コンストラクタで active=false, lifetime=0 等のデフォルト値を設定する。
    Projectile();
};
