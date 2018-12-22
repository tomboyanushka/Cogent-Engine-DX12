#pragma once
#include "stdafx.h"

class DXCore
{
public:

	static DXCore* DXCoreInstance;

	DXCore();
	~DXCore();

	//Handle to the window
	HWND hwnd = NULL;

	//name of the window
	LPCTSTR WindowName = L"DirectX12Engine";

	//title of the window
	LPCTSTR WindowTitle = L"Cogent Engine";

	//window specs
	int Width = 1920;
	int Height = 1080;

	bool FullScreen = false;

	bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

	void mainloop();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

