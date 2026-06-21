#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <iostream>

// ============================================================
// GameState — シーン間遷移に使う状態列挙
// ============================================================
// main.cpp のゲームループが各フレームで GetState() を呼び、
// 状態に応じて currentScene を切り替える。
enum class GameState
{
    Title,     // タイトル画面表示中
    Start,     // タイトル → ゲームシーンへの遷移トリガー
    Playing,   // ゲームプレイ中
    GameOver,  // タイムアップまたは HP 切れ
    GameClear, // ゴール到達
    Restart,   // ゲームシーン → タイトルシーンへの遷移トリガー
    Finished   // アプリケーション終了トリガー
};

// ============================================================
// Scene — 全シーンの純粋仮想インターフェース
// ============================================================
// 具体的なシーン（TitleScene, GameScene）はこのクラスを継承して実装する。
// main.cpp は unique_ptr<Scene> で現在のシーンを保持し、
// シーン遷移時に make_unique で新しいシーンに差し替える。
class Scene
{
public:
    // シーン開始時に 1 度だけ呼ばれる。メンバ変数の初期化はここで行う。
    virtual void Init() = 0;
    // 毎フレーム呼ばれる状態更新処理。deltaTime は秒単位の経過時間。
    virtual void Update(float deltaTIme, const InputState &inputState) = 0;
    // 毎フレーム呼ばれる描画処理。Present() の呼び出しもここで行う。
    virtual void Draw(Renderer &renderer, const InputState &inputState) = 0;
    // 現在の GameState を返す。main.cpp がシーン遷移判断に使用する。
    virtual GameState GetState() = 0;
    // 派生クラスが正しくデストラクトされるように仮想デストラクタを宣言する。
    virtual ~Scene() = default;
};
