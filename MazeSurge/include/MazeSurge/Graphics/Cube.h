#pragma once
#include "MazeSurge/Core/Core.h"

// ============================================================
// Cube — キューブの変換パラメータを保持するデータクラス
// ============================================================
// 現在のレンダリングパイプラインでは Renderer::DrawCube() に XMMATRIX を直接渡しており、
// このクラスは Renderer からは参照されていない。
// 将来的なシーングラフや汎用オブジェクト管理に使用する想定のデータホルダー。
class Cube
{
public:
    XMFLOAT3 position; // ワールド空間における中心座標
    XMFLOAT3 scale;    // 各軸方向のスケール（非均一スケールも可）
    // angle は Y 軸周りの回転（ヨー）のみ。ピッチ・ロールは対応していない。
    float angle; // Y 軸周りの回転角（ラジアン）

    // position/scale/angle を受け取るコンストラクタ。デフォルトコンストラクタは削除済み。
    Cube(XMFLOAT3 position, XMFLOAT3 scale, float angle)
        : position(position), scale(scale), angle(angle) {}
};
