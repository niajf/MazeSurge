#pragma once
#include "MazeSurge/Core/Core.h"

// ---- ボタン矩形（RECT: {left, top, right, bottom} クライアント座標）----
// GAME_EXIT_BUTTON: ゲームオーバー/クリア画面の「TITLE へ戻る」ボタン。
// 画面中央下部（x: 540〜740, y: 480〜540）に配置。
constexpr RECT GAME_EXIT_BUTTON_RECT = {540, 480, 740, 540};

// TITLE_START/EXIT_BUTTON: タイトル画面のボタン。
// ロゴの下方（y: 490〜630 あたり）に 2 段並べて配置。
constexpr RECT TITLE_START_BUTTON_RECT = {540, 540 - 50, 740, 600 - 50};
constexpr RECT TITLE_EXIT_BUTTON_RECT = {540, 620 - 50, 740, 680 - 50};

// ---- リソースパス（実行ファイルからの相対パス）----
// .spritefont は DirectXTK の makespritefont ツールで生成したバイナリフォント。
constexpr const wchar_t *UI_FONT_PATH = L"fonts/gameover.spritefont";
constexpr const wchar_t *UI_TITLE_IMAGE_PATH = L"figures/title_logo.png";
constexpr const wchar_t *UI_TITLE_BG_IMAGE_PATH = L"figures/title_background.png";
constexpr const wchar_t *UI_HOW_TO_PLAY_IMAGE_PATH = L"figures/how_to_play.png";

// ---- HOW TO PLAY ----
// HOW_TO_PLAY_DRAW_RECT: 操作説明画像を画面内に収めるための描画矩形。
// 画面四辺から一定マージンを取り、迷路ビューを部分的に覆うサイズ。
constexpr RECT HOW_TO_PLAY_DRAW_RECT = {160, 90, 1120, 630};

// HOW_TO_PLAY_DRAW_TIME: ゲーム開始から 7 秒間だけ操作説明を表示する。
// プレイヤーが操作を把握するのに十分な時間。
constexpr float HOW_TO_PLAY_DRAW_TIME = 7.0f;

// ---- HUD ----
constexpr float UI_HUD_BAR_HEIGHT = 60.0f;             // 画面上部の HUD バーの高さ（ピクセル）
constexpr XMFLOAT2 UI_HUD_HP_POS = {50.0f, 10.0f};     // HP テキストの描画位置（画面左）
constexpr XMFLOAT2 UI_HUD_CP_POS = {590.0f, 10.0f};    // チェックポイント数の描画位置（画面中央）
constexpr XMFLOAT2 UI_HUD_TIME_POS = {1020.0f, 10.0f}; // 残り時間の描画位置（画面右）
constexpr float UI_HUD_TEXT_SCALE = 0.5f;              // HUD テキストの拡大率
// HUD バーは半透明（alpha=0.78）にして背景の 3D シーンを透かせる。
constexpr XMFLOAT4 UI_HUD_BAR_COLOR = {0.08f, 0.08f, 0.12f, 0.78f};

// ---- ボタン ----
constexpr float UI_BUTTON_TEXT_SCALE = 0.5f; // ボタン内テキストの拡大率

// ---- 結果画面（GAME OVER / GAME CLEAR）オフセット ----
// 画面縦中央からの Y 方向ずらし量（負値 = 上方向）。
// タイトル文字、ランク文字、ボタンの 3 段を縦に並べるためのオフセット。
constexpr float UI_RESULT_TITLE_Y = 200.f; // タイトル（最上段）
constexpr float UI_RESULT_MIDLE_Y = 280.f; // シンクロランク（中段）

// ---- タイトルシーン配色 ----
constexpr XMFLOAT4 UI_TITLE_BG_COLOR = {0.84f, 0.84f, 0.84f, 1.0f};        // ライトグレー背景
constexpr XMFLOAT4 UI_TITLE_START_BTN_COLOR = {0.17f, 0.49f, 0.60f, 1.0f}; // ティールブルー（START）
constexpr XMFLOAT4 UI_TITLE_EXIT_BTN_COLOR = {0.64f, 0.25f, 0.41f, 1.0f};  // ダークピンク（EXIT）

// ---- ゲームオーバー画面配色 ----
// 敵の色（ホットピンク）に合わせて「脅威」を表現する。
constexpr XMFLOAT4 UI_GAME_OVER_TITLE_COLOR = {1.00f, 0.00f, 0.41f, 1.0f}; // 敵色（ホットピンク）
constexpr XMFLOAT4 UI_GAME_OVER_BTN_COLOR = {0.60f, 0.00f, 0.25f, 1.0f};   // 同色系の暗め

// ---- ゲームクリア画面配色 ----
// プレイヤーの色（シアン）に合わせて「達成感」を表現する。
constexpr XMFLOAT4 UI_GAME_CLEAR_TITLE_COLOR = {0.00f, 0.74f, 1.00f, 1.0f}; // プレイヤー色（シアン）
constexpr XMFLOAT4 UI_GAME_CLEAR_BTN_COLOR = {0.00f, 0.44f, 0.60f, 1.0f};   // 同色系の暗め

// ---- 結果画面 共通配色 ----
// ランク文字は D→S でレア感が段階的に増すよう、
// グレー（コモン）→ 緑 → 青 → 紫 → 金（レジェンダリー）の定番配色を採用する。
// S は既存のゴールデンイエローで固定し、そこから逆算して D〜A を配色。
constexpr XMFLOAT4 UI_RESULT_RANK_D_COLOR = {0.69f, 0.69f, 0.69f, 1.0f}; // グレー（コモン）
constexpr XMFLOAT4 UI_RESULT_RANK_C_COLOR = {0.30f, 0.69f, 0.31f, 1.0f}; // 緑（アンコモン）
constexpr XMFLOAT4 UI_RESULT_RANK_B_COLOR = {0.13f, 0.59f, 0.95f, 1.0f}; // 青（レア）
constexpr XMFLOAT4 UI_RESULT_RANK_A_COLOR = {0.61f, 0.15f, 0.69f, 1.0f}; // 紫（エピック）
constexpr XMFLOAT4 UI_RESULT_RANK_S_COLOR = {0.91f, 0.66f, 0.00f, 1.0f}; // 金（レジェンダリー・固定）

// ---- 結果画面 共通時間 ----
constexpr float UI_RESULT_DRAW_RANK_INREVAL = 0.2f;
constexpr float UI_RESULT_DRAW_TITLE_SEC = 1.f;
constexpr float UI_RESULT_DRAW_MIDLE_SEC = 2.f;
constexpr float UI_RESULT_DRAW_RANK_SEC = 3.f;
constexpr float UI_RESULT_DRAW_BTN_SEC = 4.f;

// オーバーレイは半透明（alpha=0.65）にして背景の 3D シーンを薄く透かせる。
constexpr XMFLOAT4 UI_RESULT_OVERLAY_COLOR = {0.96f, 0.95f, 0.92f, 0.65f};
