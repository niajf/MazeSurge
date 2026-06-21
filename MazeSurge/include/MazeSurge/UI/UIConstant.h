#pragma once
#include "MazeSurge/Core/Core.h"

// ---- ボタン矩形 ----
constexpr RECT GAME_OVER_BUTTON_RECT = {1180 - 640, 360 + 20, 1380 - 640, 360 + 60 + 20};
constexpr RECT TITLE_START_BUTTON_RECT = {540, 340, 740, 400};
constexpr RECT TITLE_EXIT_BUTTON_RECT = {540, 420, 740, 480};

// ---- リソースパス ----
constexpr const wchar_t *UI_FONT_PATH = L"fonts/gameover.spritefont";
constexpr const wchar_t *UI_TITLE_IMAGE_PATH = L"figures/title_logo.png";
constexpr const wchar_t *UI_TITLE_BG_IMAGE_PATH = L"figures/title_background.png";

// ---- HUD ----
constexpr float UI_HUD_BAR_HEIGHT = 60.0f;
constexpr XMFLOAT2 UI_HUD_HP_POS = {50.0f, 10.0f};
constexpr XMFLOAT2 UI_HUD_CP_POS = {590.0f, 10.0f};
constexpr XMFLOAT2 UI_HUD_TIME_POS = {1020.0f, 10.0f};
constexpr float UI_HUD_TEXT_SCALE = 0.5f;
constexpr XMFLOAT4 UI_HUD_BAR_COLOR = {0.08f, 0.08f, 0.12f, 0.78f};

// ---- ボタン ----
constexpr float UI_BUTTON_TEXT_SCALE = 0.5f;

// ---- 結果画面（GAME OVER / GAME CLEAR）----
constexpr float UI_RESULT_TITLE_OFFSET_Y = -90.0f;
constexpr float UI_RESULT_RANK_OFFSET_Y = -20.0f;

// ---- タイトルシーン配色（ゲームシーンの配色と統一）----
constexpr XMFLOAT4 UI_TITLE_BG_COLOR = {0.84f, 0.84f, 0.84f, 1.0f};
constexpr XMFLOAT4 UI_TITLE_START_BTN_COLOR = {0.17f, 0.49f, 0.60f, 1.0f};
constexpr XMFLOAT4 UI_TITLE_EXIT_BTN_COLOR = {0.64f, 0.25f, 0.41f, 1.0f};

// ---- ゲームオーバー画面配色 ----
constexpr XMFLOAT4 UI_GAME_OVER_TITLE_COLOR = {1.00f, 0.00f, 0.41f, 1.0f}; // 敵色（ホットピンク）
constexpr XMFLOAT4 UI_GAME_OVER_BTN_COLOR = {0.60f, 0.00f, 0.25f, 1.0f};   // 敵色の暗め

// ---- ゲームクリア画面配色 ----
constexpr XMFLOAT4 UI_GAME_CLEAR_TITLE_COLOR = {0.00f, 0.74f, 1.00f, 1.0f}; // プレイヤー色（シアン）
constexpr XMFLOAT4 UI_GAME_CLEAR_BTN_COLOR = {0.00f, 0.44f, 0.60f, 1.0f};   // プレイヤー色の暗め

// ---- 結果画面 共通配色 ----
constexpr XMFLOAT4 UI_RESULT_RANK_COLOR = {0.91f, 0.66f, 0.00f, 1.0f};     // チェックポイント色（ゴールデンイエロー）
constexpr XMFLOAT4 UI_RESULT_OVERLAY_COLOR = {0.96f, 0.95f, 0.92f, 0.65f}; // 半透明オーバーレイ