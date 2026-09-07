#pragma once

// ---- Windows ----
#ifndef UNICODE
// UNICODE を定義することで Windows API が wchar_t 版（W 接尾辞）を既定で使うようにする。
// 未定義だと char 版（A 接尾辞）が使われ、日本語パスやマルチバイト文字で問題が出る。
#define UNICODE
#endif

// NOMINMAX: windows.h が定義する min/max マクロを無効化する。
// これらのマクロは std::min/max や XMFLOAT 演算子と名前衝突を起こすため必須。
#define NOMINMAX

// WIN32_LEAN_AND_MEAN: Winsock, COM, RPC など使用しないサブシステムのヘッダーを除外し、
// インクルード時間とマクロ汚染を減らす。
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
// windowsx.h: GET_X_LPARAM / GET_Y_LPARAM マクロを提供する。
// WM_MOUSEMOVE の lParam をキャストする際に符号付き変換を正しく行うために必要。
#include <windowsx.h>
#include <stdio.h>
#include <string>

// ---- DirectX 11 ----
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
// wrl/client.h: Microsoft::WRL::ComPtr を提供する。
// COM オブジェクトを RAII で管理し、AddRef/Release の手動呼び出しを不要にする。
#include <wrl/client.h>
#include <DirectXMath.h>
// WICTextureLoader.h: DirectXTK が提供する PNG/JPEG 等の画像ファイル読み込みユーティリティ。
// 使用には CoInitializeEx による COM 初期化が事前に必要。
#include "WICTextureLoader.h"

// ---- ライブラリリンク ----
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
// dxguid.lib: __uuidof(ID3D11Texture2D) など DXGI/D3D11 の GUID 定義を提供する。
// リンクしないと未解決外部参照エラーになる。
#pragma comment(lib, "dxguid.lib")

// ---- 共通の using 宣言 ----
// ComPtr を Microsoft::WRL:: なしで記述できるようにする。
using Microsoft::WRL::ComPtr;
// XMFLOAT3, XMMatrixLookAtLH 等の DirectXMath シンボルを DirectX:: なしで使えるようにする。
using namespace DirectX;
