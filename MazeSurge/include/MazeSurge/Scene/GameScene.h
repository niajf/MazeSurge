#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Core/GameConstant.h"
#include "MazeSurge/Scene/Scene.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/ProjectilePool.h"

// ============================================================
// GameScene — ゲームプレイ・クリア・オーバーを管理するシーン
// ============================================================
// m_state の遷移: Playing → GameClear / GameOver → Restart
// 全メンバは値型（ポインタなし）のため、Init() を呼べば再利用できる。
class GameScene : public Scene
{
public:
    GameScene() = default;

    // 全サブシステムを依存順（Dungeon → Player → Camera → Projectile → Enemy）で初期化する。
    void Init() override;

    // 毎フレーム: タイムリミット・プレイヤー・カメラ・弾・敵・チェックポイント・ゴールを更新する。
    void Update(float deltaTime, const InputState &inputState) override;

    // m_state に応じて 3D シーン + HUD または リザルト画面を描画し、Present() を呼ぶ。
    void Draw(Renderer &renderer, const InputState &inputState) override;

    GameState GetState() override;

private:
    // 指定座標がボタン矩形の内側かどうかを判定する（包含判定）。
    bool IsButtonClicked(int x, int y, const RECT &button) const;

    // インゲームの更新処理（Updateが膨大になるため個別）
    void UpdateInGame(float deltaTime, const InputState &inputState);

    // 3D シーン（ダンジョン・弾・敵・プレイヤー・HUD）を描画する。
    // GameClear/GameOver 時も呼ばれ、その上にリザルト画面をオーバーレイする。
    void DrawScene3D(Renderer &renderer);

    // 準備時間の3D シーン（ダンジョン）を描画する。
    void DrawScene3DPrePare(Renderer &renderer);

    // 残り時間比率とチェックポイント取得率の平均からランク文字（S/A/B/C/D）を返す。
    // GameOver 時は無条件で 'D'。
    int GetRankInt();

    Camera m_camera;                 // 三人称俯瞰カメラ
    Player m_player;                 // プレイヤー
    Dungeon m_dungeon;               // 迷路データ・生成・描画
    EnemyManager m_enemyManager;     // 敵のスポーン・更新・描画
    ProjectilePool m_projectilePool; // 弾のプール管理

    // m_state: Playing を起点に、ゴール→GameClear、HP/時間切れ→GameOver、
    //          ボタンクリック→Restart と遷移する。
    GameState m_state;

    int m_getCheckPoint; // 取得済みチェックポイント数（ランク計算に使用）
    float m_timeLimit;   // 残り制限時間（秒）。DrawTime() に渡す値 = m_timeLimit - m_elapsedTime。

    // m_elapsedTime: ゲーム開始からの累積時間。
    // タイムリミット判定・ランクスコア計算・操作説明の非表示タイミングに使用する。
    float m_elapsedTime;
    float m_elapsedTimeHowToPlay;
    float m_elapsedTimePrepare;
};
