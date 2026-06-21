#pragma once
#include "MazeSurge/Core/Common.h"

// ウィンドウサイズ
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// ウィンドウクラス
constexpr LPCWSTR WINDOW_CLASS = L"MazeSurgeWindowClass";
constexpr LPCWSTR WINDOW_TITLE = L"MazeSurge";

// 制限時間
constexpr float TIME_LIMIT = 5.f * 60.f;

// ---- Dungeon ----
constexpr int DUNGEON_GRID_SIZE = 31;      // 迷路のサイズ
constexpr int DUNGEON_CHECKPOINT_NUM = 5;  // チェックポイントの数
constexpr float DUNGEON_WALL_SCALE = 2.0f; // 壁ブロックのスケール
constexpr float DUNGEON_GOAL_SCALE = 2.0f; // 壁ブロックのスケール
constexpr float DUNGEON_CP_SCALE = 1.4f;   // チェックポイントブロックのスケール
constexpr XMFLOAT4 DUNGEON_WALL_COLOR = {0.00f, 0.44f, 1.00f, 1.0f};
constexpr XMFLOAT4 DUNGEON_GOAL_COLOR = {0.25f, 0.80f, 0.00f, 1.0f};
constexpr XMFLOAT4 DUNGEON_CP_COLOR = {0.91f, 0.66f, 0.00f, 1.0f};

// ---- Enemy ----
constexpr float ENEMY_MOVE_SPEED = 2.5f; // 敵の移動速度
constexpr float ENEMY_CELL_SCALE = 0.8f; // 敵ブロックのスケール
constexpr XMFLOAT4 ENEMY_CELL_COLOR = {1.00f, 0.00f, 0.41f, 1.0f};

// ---- EnemyManager ----
constexpr size_t ENEMYMANAGER_POOL_SIZE = 50;
constexpr float ENEMYMANAGER_SWAWN_INTERVAL = 3.0f;
constexpr float ENEMYMANAGER_SPAWN_TIME_SCALE = 60.0f; // 経過時間に対するスポーン間隔の短縮スケール

// ---- Player ----
constexpr float PLAYER_CELL_SCALE = 0.8f;
constexpr float PLAYER_MOVE_SPEED = 6.0f;
constexpr int PLAYER_HP = 5;
constexpr XMFLOAT4 PLAYER_CELL_COLOR = {0.00f, 0.74f, 1.00f, 1.0f};

// ---- Projectile ----
constexpr float PROJECTILE_CELL_SCALE = 0.2f;
constexpr float PROJECTILE_MOVE_SPEED = 5.0f;
constexpr float PROJECTILE_LIFE_LIMIT_TIME = 10.f;
constexpr XMFLOAT4 PROJECTILE_CELL_COLOR = {1.00f, 0.48f, 0.00f, 1.0f};

// ---- ProjectilePool ----
constexpr float PROJECTILEPOOL_FIRE_INTERVAL_TIME = 0.1f;
constexpr size_t PROJECTILEPOOL_POOL_SIZE = 50;

// ---- Camera ----
constexpr float CAMERA_FOV_DEG  = 60.f;
constexpr float CAMERA_OFFSET_Y = 14.0f;
constexpr float CAMERA_OFFSET_Z = -7.0f;
constexpr float CAMERA_NEAR_CLIP = 0.1f;
constexpr float CAMERA_FAR_CLIP  = 100.0f;

// ---- Rank ----
constexpr float RANK_S_SCORE = 0.7f;
constexpr float RANK_A_SCORE = 0.6f;
constexpr float RANK_B_SCORE = 0.5f;
constexpr float RANK_C_SCORE = 0.4f;

// ---- Renerer ----
constexpr XMFLOAT4 RENDERER_FLOOR_COLOR = {0.88f, 0.86f, 0.82f, 1.0f};
constexpr XMFLOAT4 RENDERER_PLAY_BG_COLOR = {0.96f, 0.95f, 0.92f, 1.0f};
constexpr XMFLOAT3 RENDERER_LIGHT_DIR = {0.5f, -1.0f, 0.3f};
constexpr XMFLOAT3 RENDERER_LIGHT_COLOR = {1.0f, 1.0f, 0.95f};
constexpr float RENDERER_SHININESS = 64.0f;
constexpr float RENDERER_SPECULAR_INTENSITY = 0.2f;
constexpr float RENDERER_EMISSIVE_INTENSITY = 0.7f;