#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <iostream>

// シーンの基底クラス（インターフェース）
enum class GameState
{
    Title,
    Start,
    Playing,
    GameOver,
    GameClear,
    Restart,
    Finished
};

class Scene
{
public:
    virtual void Init() = 0;                                                 // シーン開始時に一度だけ呼ばれる初期化処理
    virtual void Update(float deltaTIme, const InputState &inputState) = 0;  // 毎フレーム呼ばれる状態更新処理
    virtual void Draw(Renderer &renderer, const InputState &inputState) = 0; // 毎フレーム呼ばれる描画処理
    virtual GameState GetState() = 0;                                        // 現在のゲーム状態を返す（シーン遷移の判断に使用）
    virtual ~Scene() = default;
};