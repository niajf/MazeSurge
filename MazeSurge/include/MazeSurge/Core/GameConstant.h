#pragma once
#include "MazeSurge/Core/Common.h"

// ---- ウィンドウ ----
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr LPCWSTR WINDOW_CLASS = L"MazeSurgeWindowClass";
constexpr LPCWSTR WINDOW_TITLE = L"MazeSurge";

// ---- Scene ----
constexpr float TIME_LIMIT = 5.f * 60.f; // ゲームの制限時間（秒）= 5 分
// INPUT_INVALID_TIME: シーン遷移直後の一定時間、入力を無効にする。
// ゲームシーンでのクリックがタイトルシーンに引き継がれる誤入力を防ぐ。
constexpr float INPUT_INVALID_TIME = 1.0f;

// ---- Dungeon ----
// DUNGEON_GRID_SIZE は奇数でなければならない。
// 穴掘り法（Recursive Backtracker）は外周を壁として保持するため奇数グリッドで
// 通路セルが (size-1)/2 × (size-1)/2 のパターンに収まる。
constexpr int DUNGEON_GRID_SIZE = 31;
constexpr int DUNGEON_CHECKPOINT_NUM = 5;
constexpr float DUNGEON_WALL_SCALE = 2.0f;                           // 1 セルのワールド単位サイズ（壁ブロック）
constexpr float DUNGEON_GOAL_SCALE = 2.0f;                           // ゴールブロックのスケール（壁と同サイズ）
constexpr float DUNGEON_CP_SCALE = 1.4f;                             // チェックポイントブロックのスケール（小さく目立たせる）
constexpr XMFLOAT4 DUNGEON_WALL_COLOR = {0.00f, 0.44f, 1.00f, 1.0f}; // ブルー
constexpr XMFLOAT4 DUNGEON_GOAL_COLOR = {0.25f, 0.80f, 0.00f, 1.0f}; // グリーン
constexpr XMFLOAT4 DUNGEON_CP_COLOR = {0.91f, 0.66f, 0.00f, 1.0f};   // ゴールデンイエロー

// ---- Enemy ----
constexpr float ENEMY_MOVE_SPEED = 2.5f;                           // 敵の移動速度（単位/秒）
constexpr float ENEMY_CELL_SCALE = 0.8f;                           // 敵キューブの辺長（プレイヤーと同サイズ）
constexpr XMFLOAT4 ENEMY_CELL_COLOR = {1.00f, 0.00f, 0.41f, 1.0f}; // ホットピンク

// ---- EnemyManager ----
constexpr size_t ENEMYMANAGER_POOL_SIZE = 50;       // 同時存在できる敵の上限数
constexpr float ENEMYMANAGER_SPAWN_INTERVAL = 3.0f; // 初期スポーン間隔（秒）
// SPAWN_TIME_SCALE: スポーン間隔の短縮スピードを制御する。
// 計算式: interval = SPAWN_INTERVAL / (1 + elapsedTime / SPAWN_TIME_SCALE)
// この値が小さいほど難易度の上昇が速くなる。
constexpr float ENEMYMANAGER_SPAWN_TIME_SCALE = 60.0f;

// ---- Player ----
constexpr float PLAYER_CELL_SCALE = 0.8f;                           // プレイヤーキューブの辺長
constexpr float PLAYER_MOVE_SPEED = 6.0f;                           // プレイヤーの移動速度（単位/秒）
constexpr int PLAYER_HP = 5;                                        // プレイヤーの初期 HP
constexpr XMFLOAT4 PLAYER_CELL_COLOR = {0.00f, 0.74f, 1.00f, 1.0f}; // シアン

// ---- Projectile ----
constexpr float PROJECTILE_CELL_SCALE = 0.2f;  // 弾キューブの辺長（プレイヤーより小さい）
constexpr float PROJECTILE_MOVE_SPEED = 10.0f; // 弾の移動速度（単位/秒）
// LIFE_LIMIT_TIME=10s は通常プレイで迷路の壁に当たる前に消えない十分な長さ。
// 壁衝突判定を省略しているため、寿命で消滅させている。
constexpr float PROJECTILE_LIFE_LIMIT_TIME = 10.f;
constexpr XMFLOAT4 PROJECTILE_CELL_COLOR = {1.00f, 0.48f, 0.00f, 1.0f}; // オレンジ

// ---- ProjectilePool ----
// FIRE_INTERVAL_TIME=0.1s: 1 秒間に最大 10 発の連射が可能。
constexpr float PROJECTILEPOOL_FIRE_INTERVAL_TIME = 0.1f;
constexpr size_t PROJECTILEPOOL_POOL_SIZE = 50; // 同時に存在できる弾の上限数

// ---- Camera ----
constexpr float CAMERA_FOV_DEG = 60.f; // 垂直視野角（度）。広すぎると歪みが目立つ。
// OFFSET_Y: プレイヤーの真上 14 ユニット。迷路全体が俯瞰できる高さ。
constexpr float CAMERA_OFFSET_Y = 14.0f;
// OFFSET_Z: プレイヤーより -7 ユニット（手前方向）。視点が少し傾いて奥行き感が出る。
constexpr float CAMERA_OFFSET_Z = -7.0f;
// NEAR_CLIP が小さすぎると深度バッファの精度が下がり Z ファイティングが発生する。
constexpr float CAMERA_NEAR_CLIP = 0.1f;
constexpr float CAMERA_FAR_CLIP = 100.0f;

// ---- Rank 判定スコア閾値（0.0〜1.0）----
// スコア = (残り時間比率 + チェックポイント取得率) / 2
constexpr float RANK_S_SCORE = 0.7f; // S: スコア 70% 以上
constexpr float RANK_A_SCORE = 0.6f; // A: スコア 60% 以上
constexpr float RANK_B_SCORE = 0.5f; // B: スコア 50% 以上
constexpr float RANK_C_SCORE = 0.4f; // C: スコア 40% 以上
                                     // D: スコア 40% 未満 または GameOver

// ---- Renderer ----
constexpr XMFLOAT4 RENDERER_FLOOR_COLOR = {0.88f, 0.86f, 0.82f, 1.0f};   // ライトグレー（床）
constexpr XMFLOAT4 RENDERER_PLAY_BG_COLOR = {0.96f, 0.95f, 0.92f, 1.0f}; // オフホワイト（背景）
// LIGHT_DIR: 右斜め上方からの方向光。normalize は HLSL シェーダー側で行う。
constexpr XMFLOAT3 RENDERER_LIGHT_DIR = {0.5f, -1.0f, 0.3f};
constexpr XMFLOAT3 RENDERER_LIGHT_COLOR = {1.0f, 1.0f, 0.95f}; // 白に近い暖色
// SHININESS=64: Phong モデルの指数。大きいほどスペキュラーが鋭く小さくなる。
constexpr float RENDERER_SHININESS = 64.0f;
constexpr float RENDERER_SPECULAR_INTENSITY = 0.2f; // スペキュラー反射の強度
// EMISSIVE_INTENSITY=0.7: 環境光の強度。0 だと影の部分が真っ黒になる。
constexpr float RENDERER_EMISSIVE_INTENSITY = 0.7f;

namespace Audio
{
    constexpr const wchar_t *GAME_BGM_PATH = L"Sounds/FinalCorridorDash.mp3";
    constexpr const wchar_t *TITLE_BGM_PATH = L"Sounds/PendingAdventure.mp3";
    constexpr const wchar_t *CLEAR_BGM_PATH = L"Sounds/ClearTheMaze.mp3";
    constexpr const wchar_t *OVER_BGM_PATH = L"Sounds/ZeroCreditLeft.mp3";
    constexpr const wchar_t *HIT_ENEMY_SE_PATH = L"Sounds/8bitDamage10.mp3";
    constexpr const wchar_t *HIT_PLAYER_SE_PATH = L"Sounds/8bitAlert3.mp3";
    constexpr const wchar_t *GET_CP_SE_PATH = L"Sounds/8bitGet8.mp3";
    constexpr const wchar_t *RANK_UP_SE_PATH = L"Sounds/8bitSelect2.mp3";
    constexpr const wchar_t *SELECT_SE_PATH = L"Sounds/8bitGet6.mp3";
}