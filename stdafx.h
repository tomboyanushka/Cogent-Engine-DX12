#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

//Handle to the window
HWND hwnd = NULL;

//name of the window
LPCTSTR WindowName = L"DirectX12Engine";

//title of the window
LPCTSTR WindowTitle = L"Cogent Engine";

//window specs
int Width = 800;
int Height = 600;

bool FullScreen = false;

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

void mainloop();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);