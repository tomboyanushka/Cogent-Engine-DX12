#include "stdafx.h"
#include "DXCore.h"

DXCore dxCore;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (!dxCore.InitializeWindow(hInstance, nShowCmd, dxCore.Width, dxCore.Height, dxCore.FullScreen))
	{
		MessageBox(0, L"Window Initialization failed", L"Error", MB_OK);
		return 0;
	}
	dxCore.InitializeDirectX();
	dxCore.mainloop();



	return 0;
}





