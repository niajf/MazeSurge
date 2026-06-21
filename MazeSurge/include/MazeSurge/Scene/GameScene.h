#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Core/GameConstant.h"
#include "MazeSurge/Scene/Scene.h"
#include "MazeSurge/Graphics/Camera.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/ProjectilePool.h"

class GameScene : public Scene
{
public:
    GameScene() = default;
    void Init() override;
    void Update(float deltaTime, const InputState &inputState) override;
    void Draw(Renderer &renderer, const InputState &inputState) override;
    GameState GetState() override;

private:
    bool IsButtonClicked(int x, int y, const RECT &button) const; // 指定座標がボタン矩形内にあるかを返す
    void DrawScene3D(Renderer &renderer);                         // ダンジョン・プレイヤー・敵・弾を3D描画する
    char GetRankChar();                                           // チェックポイント取得率からランク文字（S/A/B/C/D）を返す

    Camera m_camera;                 // 三人称視点カメラ
    Player m_player;                 // プレイヤー
    Dungeon m_dungeon;               // 迷路データと描画
    EnemyManager m_enemyManager;     // 敵のスポーン・更新・描画を管理する
    ProjectilePool m_projectilePool; // 弾のプール管理
    GameState m_state;               // 現在のゲーム状態（Playing / GameOver / GameClear 等）

    int m_getCheckPoint; // 取得済みチェックポイント数
    float m_timeLimit;   // 残り制限時間（秒）
    float m_elapsedTime; // ゲーム開始からの経過時間（スポーン間隔調整に使用）
};
