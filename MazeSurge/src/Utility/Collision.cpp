#include "MazeSurge/Utility/Collision.h"

// AABB（Axis-Aligned Bounding Box）の 2D 重なり判定。
// ゲームのオブジェクトは Y 方向に高さが異なるが、衝突判定は XZ 平面上の 2D で行う。
// これはトップダウン視点のゲームでは一般的な簡略化であり、
// 高さ方向の厳密な判定を省くことで計算コストを下げている。
bool Collision::checkAABB(const BBOX &box1, const BBOX &box2)
{
    // 2 つの AABB が重なる条件:
    //   X 軸: box1.minX <= box2.maxX かつ box1.maxX >= box2.minX
    //   Z 軸: box1.minZ <= box2.maxZ かつ box1.maxZ >= box2.minZ
    // 両軸とも分離がなければ重なっている（Separating Axis Theorem の 2D 版）。
    if (box1.minX <= box2.maxX && box1.maxX >= box2.minX &&
        box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ)
    {
        return true;
    }

    return false;
}
