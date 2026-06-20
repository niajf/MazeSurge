#pragma once
#include "MazeSurge/Core/Common.h"

// ウィンドウサイズ
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// ウィンドウクラス
constexpr LPCWSTR WINDOW_CLASS = L"MazeSurgeWindowClass";
constexpr LPCWSTR WINDOW_TITLE = L"MazeSurge";

// 制限時間
constexpr float TIME_LIMIT = 7.f * 60.f;

// ---- Dungeon ----
constexpr int DUNGEON_GRID_SIZE = 11;      // 迷路のサイズ
constexpr int DUNGEON_CHECKPOINT_NUM = 5;  // チェックポイントの数
constexpr float DUNGEON_CELL_SCALE = 2.0f; // 壁ブロックのスケール

// ---- Enemy ----
constexpr float ENEMY_MOVE_SPEED = 2.5f; // 敵の移動速度
constexpr float ENEMY_CELL_SCALE = 0.8f; // 敵ブロックのスケール

// ---- EnemyManager ----
constexpr size_t ENEMYMANAGER_POOL_SIZE = 50;
constexpr float ENEMYMANAGER_SWAWN_INTERVAL = 3.0f;

// ---- Player ----
constexpr float PLAYER_CELL_SCALE = 0.8f;
constexpr float PLAYER_MOVE_SPEED = 6.0f;
constexpr int PLAYER_HP = 6;

// ---- Projectile ----
constexpr float PROJECTILE_CELL_SCALE = 0.2f;
constexpr float PROJECTILE_MOVE_SPEED = 5.0f;
constexpr float PROJECTILE_LIFE_LIMIT_TIME = 10.f;

// ---- ProjectilePool ----
constexpr float PROJECTILEPOOL_FIRE_INTERVAL_TIME = 0.1f;
constexpr size_t PROJECTILEPOOL_POOL_SIZE = 50;

// ---- Camera ----
constexpr float CAMERA_FOV_DEG = 60.f;
constexpr float CAMERA_OFFSET_Y = 14.0f;
constexpr float CAMERA_OFFSET_Z = -7.0f;
