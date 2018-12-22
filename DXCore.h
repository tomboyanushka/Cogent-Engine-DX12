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

	//HRESULT InitializeDirectX();

	bool InitializeDirectX();
	void Uodate();
	void UpdatePipeline();
	void Render();
	void Cleanup();
	void WaitForPreviousFrame();

protected:

	// direct3d stuff
	const int frameBufferCount = FRAMEBUFFERCOUNT;
	ID3D12Device* device;
	IDXGISwapChain3* swapChain;
	ID3D12CommandQueue* commandQueue;
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	ID3D12Resource* renderTargets[FRAMEBUFFERCOUNT];
	ID3D12CommandAllocator* commandAllocator[FRAMEBUFFERCOUNT];
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence[FRAMEBUFFERCOUNT];
	HANDLE fenceEvent;
	UINT64 fenceValue[FRAMEBUFFERCOUNT];
	int frameIndex;
	int rtvDescriptorSize;
	
};

