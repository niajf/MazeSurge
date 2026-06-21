#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Scene/Scene.h"
#include "MazeSurge/Graphics/Renderer.h"

// ============================================================
// TitleScene — タイトル画面のシーン
// ============================================================
// 状態遷移: Title（初期） → Start（STARTボタン） / Finished（EXITボタン）
// ゲームシーンからの復帰時にも Init() で状態をリセットして再利用される。
class TitleScene : public Scene
{
public:
    TitleScene() = default;
    // m_state を Title に、m_elapsedTime を 0 に初期化する。
    void Init();
    // m_elapsedTime を累積し、INPUT_INVALID_TIME 経過後にボタン入力を受け付ける。
    void Update(float deltaTime, const InputState &inputState) override;
    // タイトル背景・ロゴ・ボタンを描画して Present() を呼ぶ。
    void Draw(Renderer &renderer, const InputState &inputState) override;
    GameState GetState() { return m_state; }

private:
    // 指定座標がボタン矩形の内側かどうかを判定する（GameScene::IsButtonClicked と同実装）。
    bool IsButtonClicked(int x, int y, const RECT &button) const;

    GameState m_state; // 現在の状態（Title / Start / Finished）
    // m_elapsedTime: シーン開始からの経過時間。
    // INPUT_INVALID_TIME（0.5s）未満の間は入力を無視することで、
    // ゲームシーンでのボタン離し操作がタイトルに引き継がれる誤入力を防ぐ。
    float m_elapsedTime;
};
