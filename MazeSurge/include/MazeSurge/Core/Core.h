#pragma once

// プロジェクト全体で必要な基本ヘッダーをまとめてインクルードするアグリゲーションヘッダー。
// ゲームコードのほとんどは Core.h 1 つをインクルードすれば事足りる。
#include "MazeSurge/Core/Common.h"        // Windows / DirectX / COM の共通設定
#include "MazeSurge/Core/Types.h"         // Vertex / ConstantBuffer / BBOX / InputState
#include "MazeSurge/Core/GameConstant.h"  // ゲームロジック・描画定数
#include "MazeSurge/Audio/SoundManager.h" // サウンド系

// 頻繁にインクルードが必要な標準ライブラリを記述
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
