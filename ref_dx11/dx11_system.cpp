/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/*
ref_dx11
2017 Bleeding Eye Studios
*/

#include "dx11_local.hpp"

dx11::System* sys = nullptr;

D3D_FEATURE_LEVEL FeatureLevelForString(std::string featureLevelString)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;

	std::map<std::string, D3D_FEATURE_LEVEL> featureLevelMap;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_12_1)] = D3D_FEATURE_LEVEL_12_1;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_12_0)] = D3D_FEATURE_LEVEL_12_0;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_11_1)] = D3D_FEATURE_LEVEL_11_1;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_11_0)] = D3D_FEATURE_LEVEL_11_0;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_10_1)] = D3D_FEATURE_LEVEL_10_1;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_10_0)] = D3D_FEATURE_LEVEL_10_0;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_9_3)] = D3D_FEATURE_LEVEL_9_3;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_9_2)] = D3D_FEATURE_LEVEL_9_2;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_9_1)] = D3D_FEATURE_LEVEL_9_1;

	auto search = featureLevelMap.find(featureLevelString);
	if (search != featureLevelMap.end()) 
	{
		featureLevel = search->second;
	}

	return featureLevel;
}

void dx11::System::FillFeatureLevelArray(void)
{
	featureLevelArray[0] = D3D_FEATURE_LEVEL_12_1;
	featureLevelArray[1] = D3D_FEATURE_LEVEL_12_0;
	featureLevelArray[2] = D3D_FEATURE_LEVEL_11_1;
	featureLevelArray[3] = D3D_FEATURE_LEVEL_11_0;
	featureLevelArray[4] = D3D_FEATURE_LEVEL_10_1;
	featureLevelArray[5] = D3D_FEATURE_LEVEL_10_0;
	featureLevelArray[6] = D3D_FEATURE_LEVEL_9_3;
	featureLevelArray[7] = D3D_FEATURE_LEVEL_9_2;
	featureLevelArray[8] = D3D_FEATURE_LEVEL_9_1;
}

dx11::System::System()
{
	hInstance = nullptr;
	wndProc = nullptr;
	hWnd = nullptr;
	FillFeatureLevelArray();

	driverType = D3D_DRIVER_TYPE_NULL;
	featureLevel = D3D_FEATURE_LEVEL_12_1;

	d3dDevice = nullptr;
	d3dDevice1 = nullptr;
	ImmediateContext = nullptr;
	ImmediateContext1 = nullptr;
	SwapChain = nullptr;
	SwapChain1 = nullptr;
	RenderTargetView = nullptr;

	d3dInitialized = false;
}

dx11::System::~System()
{
	Shutdown();
}

void dx11::System::BeginRegistration()
{
	if (!inRegistration)
	{
		BeginUpload();

		inRegistration = true;
	}
}

void dx11::System::EndRegistration()
{
	if (inRegistration)
	{
		inRegistration = false;

		EndUpload();
	}
}

void dx11::System::BeginUpload()
{
	/*if (resourceUpload == nullptr)
	{
		resourceUpload = new DirectX::ResourceUploadBatch(ref->sys->d3dDevice);
	}*/

	if (!uploadBatchOpen)
	{
		//resourceUpload->Begin();

		uploadBatchOpen = true;
	}
}

void dx11::System::EndUpload()
{
	// Only flush the upload if the batch is open AND we are not in registration mode
	if (uploadBatchOpen && (!inRegistration))
	{
		// Upload the resources to the GPU.
		//auto uploadResourcesFinished = resourceUpload->End(ref->sys->cmdQueue);

		// Wait for the upload thread to terminate
		//uploadResourcesFinished.wait();

		uploadBatchOpen = false;
	}
}


/*
** VID_CreateWindow
*/
bool dx11::System::VID_CreateWindow()
{
	RECT			r			= {};
	int				stylebits	= 0;
	int				x			= 0,
					y			= 0,
					w			= 0,
					h			= 0;
	int				exstyle		= 0;
	unsigned int	width		= ref->cvars->r_customWidth->UInt(),
					height		= ref->cvars->r_customHeight->UInt();
	bool			fullscreen	= ref->cvars->vid_fullscreen->Bool();

	/* Register the frame class */
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = 0;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_GRAYTEXT + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClass(&wndClass))
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't register window class");
	}

	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP | WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		stylebits = WINDOW_STYLE;
	}

	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;

	AdjustWindowRect(&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		x = ref->cvars->vid_xPos->UInt();
		y = ref->cvars->vid_yPos->UInt();
	}

	hWnd = CreateWindowEx(
		exstyle,
		WINDOW_CLASS_NAME,
		"Quake 2",
		stylebits,
		x, y, w, h,
		NULL,
		NULL,
		hInstance,
		nullptr);

	if (!hWnd)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't create window");
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	// let the sound and input subsystems know about the new window
	ref->client->Vid_NewWindow(width, height);

	return true;
}

void dx11::System::VID_DestroyWindow()
{
	if (hWnd != nullptr)
	{
		DestroyWindow(hWnd);
		hWnd = nullptr;
	}
}


bool dx11::System::Initialize(HINSTANCE hInstance, WNDPROC wndProc)
{
	if (d3dInitialized)
	{
		D3D_Shutdown();
	}

	if (hWnd != nullptr)
	{
		VID_DestroyWindow();
	}

	this->hInstance = hInstance;
	this->wndProc = wndProc;

	if (!VID_CreateWindow())
	{
		return false;
	}

	if (!D3D_InitDevice())
	{
		return false;
	}

	return true;
}

void dx11::System::Shutdown()
{
	if (d3dInitialized)
	{
		D3D_Shutdown();
	}

	if (hWnd != nullptr)
	{
		VID_DestroyWindow();
	}
}

void dx11::System::AppActivate(bool active)
{
	if (active)
	{
		SetForegroundWindow(hWnd);
		ShowWindow(hWnd, SW_RESTORE);
	}
	else
	{
		if (ref->cvars->vid_fullscreen->Bool())
		{
			ShowWindow(hWnd, SW_MINIMIZE);
		}
	}
}

bool dx11::System::D3D_InitDevice()
{
	HRESULT hr = S_OK;
	RECT rc = {};
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &ImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.1 runtime will not recognize D3D_FEATURE_LEVEL_12_x, so try without them
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[2], numFeatureLevels - 2, D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &ImmediateContext);

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1+ so we need to retry without it
				hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[3], numFeatureLevels - 3, D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &ImmediateContext);
			}
		}

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		return false;
	}


	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory5* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;

		hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;

			hr = dxgiDevice->GetAdapter(&adapter);

			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	if (FAILED(hr))
	{
		return false;
	}

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));

	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = d3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&d3dDevice1));

		if (SUCCEEDED(hr))
		{
			(void)ImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&ImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(d3dDevice, hWnd, &sd, nullptr, nullptr, &SwapChain1);

		if (SUCCEEDED(hr))
		{
			hr = SwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&SwapChain));
		}

		dxgiFactory2->Release();
		dxgiFactory2 = nullptr;
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(d3dDevice, &sd, &SwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();
	dxgiFactory = nullptr;

	if (FAILED(hr))
	{
		return false;
	}

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

	if (FAILED(hr))
	{
		return false;
	}


	hr = d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);

	pBackBuffer->Release();

	if (FAILED(hr))
	{
		return false;
	}

	ImmediateContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	ImmediateContext->RSSetViewports(1, &vp);

	return true;
}

void dx11::System::D3D_Shutdown()
{
	if (ImmediateContext)
	{
		ImmediateContext->ClearState();
	}
	
	if (RenderTargetView) 
	{
		RenderTargetView->Release();
		RenderTargetView = nullptr;
	}

	if (SwapChain1)
	{ 
		SwapChain1->Release(); 
	SwapChain1 = nullptr;
	}

	if (SwapChain) 
	{ 
		SwapChain->Release(); 
		SwapChain = nullptr; 
	}

	if (ImmediateContext1) 
	{ 
		ImmediateContext1->Release(); 
		ImmediateContext1 = nullptr; 
	}

	if (ImmediateContext) 
	{
		ImmediateContext->Release();
		ImmediateContext = nullptr;
	}

	if (d3dDevice1)
	{
		d3dDevice1->Release();
		d3dDevice1 = nullptr;
	}

	if (d3dDevice)
	{
		d3dDevice->Release();
		d3dDevice = nullptr;
	}

	d3dInitialized = false;
}