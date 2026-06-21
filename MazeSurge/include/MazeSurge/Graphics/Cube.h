#pragma once
#include "MazeSurge/Core/Core.h"

class Cube
{
public:
    XMFLOAT3 position; // ワールド空間における中心座標
    XMFLOAT3 scale;    // 各軸方向のスケール（辺の長さ）
    float angle;       // Y軸周りの回転角（ラジアン）

    // position/scale/angle を直接受け取る唯一のコンストラクタ
    Cube(XMFLOAT3 position, XMFLOAT3 scale, float angle) : position(position), scale(scale), angle(angle) {}
};