#pragma once

// プロジェクト全体で必要な基本ヘッダーをまとめてインクルードするアグリゲーションヘッダー。
// ゲームコードのほとんどは Core.h 1 つをインクルードすれば事足りる。
// 追加のシステムヘッダー（<vector> 等）が必要なファイルは Core.h とは別に追記する。
#include "MazeSurge/Core/Common.h"       // Windows / DirectX / COM の共通設定
#include "MazeSurge/Core/Types.h"        // Vertex / ConstantBuffer / BBOX / InputState
#include "MazeSurge/Core/GameConstant.h" // ゲームロジック・描画定数
