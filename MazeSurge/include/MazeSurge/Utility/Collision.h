#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Collision — 衝突判定ユーティリティ（静的メソッドのみ）
// ============================================================
// インスタンス化は不要。全メソッドが static で、状態を持たない純粋な関数群。
class Collision
{
public:
    // 2 つの AABB（Axis-Aligned Bounding Box）が重なっているかを返す。
    // 判定は XZ 平面上の 2D に限定する（Y 軸方向は無視）。
    // ゲーム内の全オブジェクトは同一の地面高度に存在するため、この簡略化で十分。
    // 実装: Separating Axis Theorem（分離軸定理）の 2D 版。
    //   X 軸に分離軸がなく、かつ Z 軸にも分離軸がない場合に重なりと判断する。
    static bool checkAABB(const BBOX &box1, const BBOX &box2);
};
