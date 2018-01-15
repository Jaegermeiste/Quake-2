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
	m_featureLevelArray[0] = D3D_FEATURE_LEVEL_12_1;
	m_featureLevelArray[1] = D3D_FEATURE_LEVEL_12_0;
	m_featureLevelArray[2] = D3D_FEATURE_LEVEL_11_1;
	m_featureLevelArray[3] = D3D_FEATURE_LEVEL_11_0;
	m_featureLevelArray[4] = D3D_FEATURE_LEVEL_10_1;
	m_featureLevelArray[5] = D3D_FEATURE_LEVEL_10_0;
	m_featureLevelArray[6] = D3D_FEATURE_LEVEL_9_3;
	m_featureLevelArray[7] = D3D_FEATURE_LEVEL_9_2;
	m_featureLevelArray[8] = D3D_FEATURE_LEVEL_9_1;
}

dx11::System::System()
{
	m_hInstance = nullptr;
	m_wndProc = nullptr;
	ZeroMemory(&m_wndClass, sizeof(WNDCLASS));
	m_hWnd = nullptr;
	FillFeatureLevelArray();

	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_12_1;

	m_d3dDevice = nullptr;
	m_d3dDevice1 = nullptr;
	m_immediateContext = nullptr;
	m_immediateContext1 = nullptr;
	m_swapChain = nullptr;
	m_swapChain1 = nullptr;
	m_backBufferRTV = nullptr;

	m_d3dInitialized = false;
}

dx11::System::~System()
{
	Shutdown();
}

void dx11::System::BeginRegistration()
{
	if (!m_inRegistration)
	{
		BeginUpload();

		m_inRegistration = true;
	}
}

void dx11::System::EndRegistration()
{
	if (m_inRegistration)
	{
		m_inRegistration = false;

		EndUpload();
	}
}

void dx11::System::BeginUpload()
{
	/*if (resourceUpload == nullptr)
	{
		resourceUpload = new DirectX::ResourceUploadBatch(ref->sys->d3dDevice);
	}*/

	if (!m_uploadBatchOpen)
	{
		//resourceUpload->Begin();

		m_uploadBatchOpen = true;
	}
}

void dx11::System::EndUpload()
{
	// Only flush the upload if the batch is open AND we are not in registration mode
	if (m_uploadBatchOpen && (!m_inRegistration))
	{
		// Upload the resources to the GPU.
		//auto uploadResourcesFinished = resourceUpload->End(ref->sys->cmdQueue);

		// Wait for the upload thread to terminate
		//uploadResourcesFinished.wait();

		m_uploadBatchOpen = false;
	}
}

void dx11::System::BeginFrame(void)
{
	// Clear immediate context
	if (m_immediateContext)
	{
		if (m_backBufferRTV)
		{
			// clear the back buffer to a deep blue
			m_immediateContext->ClearRenderTargetView(m_backBufferRTV, DirectX::Colors::Blue);
		}

		if (m_DepthStencilView)
		{
			// Clear the depth buffer
			m_immediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}

	// Clear 2D deferred
	if (m_2DdeferredContext)
	{
		// Clear the GUI Overlay buffer to transparent
		m_2DdeferredContext->ClearRenderTargetView(m_2DoverlayRTV, DirectX::Colors::Transparent);
	}

	// Clear 3D deferred
	/*if (m_3DdeferredContext)
	{

	}*/
}

void dx11::System::RenderFrame(refdef_t * fd)
{
	// Draw 3D

	// Draw 2D
	if (m_2DcommandList)
	{
		m_immediateContext->ExecuteCommandList(m_2DcommandList, TRUE);
	}
}

void dx11::System::EndFrame(void)
{
	if (m_swapChain)
	{
		// Switch the back buffer and the front buffer
		m_swapChain->Present(0, 0);
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
	m_wndClass.style = CS_HREDRAW | CS_VREDRAW;
	m_wndClass.lpfnWndProc = m_wndProc;
	m_wndClass.cbClsExtra = 0;
	m_wndClass.cbWndExtra = 0;
	m_wndClass.hInstance = m_hInstance;
	m_wndClass.hIcon = 0;
	m_wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//m_wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_GRAYTEXT + 1);
	m_wndClass.lpszMenuName = nullptr;
	m_wndClass.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClass(&m_wndClass))
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

	m_hWnd = CreateWindowEx(
		exstyle,
		WINDOW_CLASS_NAME,
		"Quake 2",
		stylebits,
		x, y, w, h,
		NULL,
		NULL,
		m_hInstance,
		nullptr);

	if (!m_hWnd)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't create window");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	// let the sound and input subsystems know about the new window
	dx11::ref->client->Vid_NewWindow(width, height);

	return true;
}

void dx11::System::VID_DestroyWindow()
{
	if (m_hWnd != nullptr)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}


bool dx11::System::Initialize(HINSTANCE hInstance, WNDPROC wndProc)
{
	if (m_d3dInitialized)
	{
		D3D_Shutdown();
	}

	if (m_hWnd != nullptr)
	{
		VID_DestroyWindow();
	}

	m_hInstance = hInstance;
	m_wndProc = wndProc;

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
	if (m_d3dInitialized)
	{
		D3D_Shutdown();
	}

	if (m_hWnd != nullptr)
	{
		VID_DestroyWindow();
	}
}

void dx11::System::AppActivate(bool active)
{
	if (active)
	{
		SetForegroundWindow(m_hWnd);
		ShowWindow(m_hWnd, SW_RESTORE);
	}
	else
	{
		if (ref->cvars->vid_fullscreen->Bool())
		{
			ShowWindow(m_hWnd, SW_MINIMIZE);
		}
	}
}

bool dx11::System::D3D_InitDevice()
{
	HRESULT hr = S_OK;
	RECT rc = {};
	UINT createDeviceFlags = 0;

	GetClientRect(m_hWnd, &rc);
	m_windowWidth = rc.right - rc.left;
	m_windowHeight = rc.bottom - rc.top;

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
		m_driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.1 runtime will not recognize D3D_FEATURE_LEVEL_12_x, so try without them
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[2], numFeatureLevels - 2, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);

			if (hr == E_INVALIDARG)
			{
				// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1+ so we need to retry without it
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[3], numFeatureLevels - 3, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);
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

		hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;

			hr = dxgiDevice->GetAdapter(&adapter);

			if (SUCCEEDED(hr))
			{
				DXGI_ADAPTER_DESC adapterDesc;
				ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));
				hr = adapter->GetDesc(&adapterDesc);

				if (SUCCEEDED(hr))
				{
					if ((adapterDesc.VendorId == 0x1414) && (adapterDesc.DeviceId == 0x8c))
					{
						// Microsoft Basic Render Driver
						ref->client->Con_Printf(PRINT_ALL, "WARNING: Microsoft Basic Render Driver is active.\n Performance of this application may be unsatisfactory.\n Please ensure that your video card is Direct3D10/11 capable\n and has the appropriate driver installed.");
					}
				}

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
		hr = m_d3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_d3dDevice1));

		if (SUCCEEDED(hr))
		{
			(void)m_immediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_immediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
		swapChainDesc.Width = m_windowWidth;
		swapChainDesc.Height = m_windowHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Stereo = FALSE;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice, m_hWnd, &swapChainDesc, nullptr, nullptr, &m_swapChain1);

		if (SUCCEEDED(hr))
		{
			hr = m_swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_swapChain));
		}

		dxgiFactory2->Release();
		dxgiFactory2 = nullptr;
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = m_windowWidth;
		swapChainDesc.BufferDesc.Height = m_windowHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = m_hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = dxgiFactory->CreateSwapChain(m_d3dDevice, &swapChainDesc, &m_swapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();
	dxgiFactory = nullptr;

	if (FAILED(hr))
	{
		return false;
	}

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_backBufferRTV);

	pBackBuffer->Release();

	if (FAILED(hr))
	{
		return false;
	}

	m_immediateContext->OMSetRenderTargets(1, &m_backBufferRTV, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (FLOAT)m_windowWidth;
	viewport.Height = (FLOAT)m_windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	m_immediateContext->RSSetViewports(1, &viewport);

	return true;
}

/*
http://rastertek.com/dx11tut11.html
*/
bool dx11::System::D3D_Init2DOverlay()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = m_windowWidth;
	textureDesc.Height = m_windowHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	hr = m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_2DrenderTargetTexture);
	if (FAILED(hr))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	hr = m_d3dDevice->CreateRenderTargetView(m_2DrenderTargetTexture, &renderTargetViewDesc, &m_2DoverlayRTV);
	if (FAILED(hr))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = m_d3dDevice->CreateShaderResourceView(m_2DrenderTargetTexture, &shaderResourceViewDesc, &m_2DshaderResourceView);
	if (FAILED(hr))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_2DdeferredContext->OMSetRenderTargets(1, &m_2DoverlayRTV, nullptr);

	// Create an orthographic projection matrix for 2D rendering.
	m_2DorthographicMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), ref->cvars->zNear2D->Float(), ref->cvars->zFar2D->Float());

	return true;
}

void dx11::System::D3D_Shutdown()
{
	if (m_immediateContext)
	{
		m_immediateContext->ClearState();
	}

	if (m_swapChain)
	{
		// switch to windowed mode to allow proper cleanup
		m_swapChain->SetFullscreenState(FALSE, nullptr);   
	}
	
	if (m_backBufferRTV) 
	{
		m_backBufferRTV->Release();
		m_backBufferRTV = nullptr;
	}

	if (m_swapChain1)
	{ 
		m_swapChain1->Release();
		m_swapChain1 = nullptr;
	}

	if (m_swapChain) 
	{ 
		m_swapChain->Release();
		m_swapChain = nullptr;
	}

	if (m_2DdeferredContext)
	{
		m_2DdeferredContext->Release();
		m_2DdeferredContext = nullptr;
	}

	if (m_2DshaderResourceView)
	{
		m_2DshaderResourceView->Release();
		m_2DshaderResourceView = 0;
	}

	if (m_2DoverlayRTV)
	{
		m_2DoverlayRTV->Release();
		m_2DoverlayRTV = 0;
	}

	if (m_2DrenderTargetTexture)
	{
		m_2DrenderTargetTexture->Release();
		m_2DrenderTargetTexture = 0;
	}

	if (m_immediateContext1)
	{ 
		m_immediateContext1->Release();
		m_immediateContext1 = nullptr;
	}

	if (m_immediateContext)
	{
		m_immediateContext->Release();
		m_immediateContext = nullptr;
	}

	if (m_d3dDevice1)
	{
		m_d3dDevice1->Release();
		m_d3dDevice1 = nullptr;
	}

	if (m_d3dDevice)
	{
		m_d3dDevice->Release();
		m_d3dDevice = nullptr;
	}

	m_d3dInitialized = false;
}