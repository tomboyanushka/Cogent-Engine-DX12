#include "DXCore.h"


DXCore * DXCore::DXCoreInstance = 0;

DXCore::DXCore()
{
}


DXCore::~DXCore()
{
}

bool DXCore::InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{

	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}
	//struct for description of window
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DXCore::WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(
		NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}


	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

void DXCore::mainloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//run game code
			Render();
		}
	}
}

LRESULT DXCore::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (MessageBox(0, L"Sure you want to quit?", L"Quitting", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool DXCore::InitializeDirectX()
{
	HRESULT hr;

	//Creating the device==========================================
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}

		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}
	
	if (!adapterFound)
	{
		return false;
	}

	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

	if (FAILED(hr))
	{
		return false;
	}

	//Create the command queue============================================
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
	{
		return false;
	}

	//Create the Swap Chain==============================================
	DXGI_MODE_DESC backbufferDesc = {};
	backbufferDesc.Width = Width;
	backbufferDesc.Height = Height;
	backbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc = backbufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !FullScreen;

	IDXGISwapChain* tempSwapChain;
	dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	//Create Descripter Heap for Back Buffer RTV=============================
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	
	//create RTV for each buffer
	for (int i = 0; i < frameBufferCount; ++i)
	{
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr))
		{
			return false;
		}

		device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	//Create Command Allocators======================================================
	//number of command allocators is equal to the number of buffers
	for (int i = 0; i < frameBufferCount; ++i)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr))
		{
			return false;
		}
	}

	//Creating the command list=======================================================
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		return false;
	}
	commandList->Close();

	//Creating the fences and fence event
	for (int i = 0; i < frameBufferCount; ++i)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			return false;
		}
		fenceValue[i] = 0;
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent = nullptr)
	{
		return false;
	}

	return true;
}

void DXCore::Update()
{
	//update app logic
}

void DXCore::UpdatePipeline()
{
	HRESULT hr;

	WaitForPreviousFrame();

	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}
	hr = commandList->Reset(commandAllocator[frameIndex], NULL);
	if (FAILED(hr))
	{
		Running = false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.3f, 0.2f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}

}

void DXCore::Render()
{
	HRESULT hr;
	
	UpdatePipeline();

	ID3D12CommandList* ppCommandLists[] = { commandList };

	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	// present the current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		Running = false;
	}
}

void DXCore::Cleanup()
{
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
	{
		swapChain->SetFullscreenState(false, NULL);
	}

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};
}

void DXCore::WaitForPreviousFrame()
{
	HRESULT hr;
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue[frameIndex]++;

}
