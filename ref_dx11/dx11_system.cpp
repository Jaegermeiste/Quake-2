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

std::string StringForFeatureLevel(D3D_FEATURE_LEVEL  featureLevel)
{
	std::string featureLevelString = "";

	std::map<D3D_FEATURE_LEVEL, std::string> featureLevelMap;
	featureLevelMap[D3D_FEATURE_LEVEL_12_1] = STR(D3D_FEATURE_LEVEL_12_1);
	featureLevelMap[D3D_FEATURE_LEVEL_12_0] = STR(D3D_FEATURE_LEVEL_12_0);
	featureLevelMap[D3D_FEATURE_LEVEL_11_1] = STR(D3D_FEATURE_LEVEL_11_1);
	featureLevelMap[D3D_FEATURE_LEVEL_11_0] = STR(D3D_FEATURE_LEVEL_11_0);
	featureLevelMap[D3D_FEATURE_LEVEL_10_1] = STR(D3D_FEATURE_LEVEL_10_1);
	featureLevelMap[D3D_FEATURE_LEVEL_10_0] = STR(D3D_FEATURE_LEVEL_10_0);
	featureLevelMap[D3D_FEATURE_LEVEL_9_3] = STR(D3D_FEATURE_LEVEL_9_3);
	featureLevelMap[D3D_FEATURE_LEVEL_9_2] = STR(D3D_FEATURE_LEVEL_9_2);
	featureLevelMap[D3D_FEATURE_LEVEL_9_1] = STR(D3D_FEATURE_LEVEL_9_1);

	auto search = featureLevelMap.find(featureLevel);
	if (search != featureLevelMap.end())
	{
		featureLevelString = search->second;
	}

	return featureLevelString;
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
	//BOOST_LOG_NAMED_SCOPE("System");
	LOG_FUNC();

	LOG(info) << "Initializing";

	m_hInstance = nullptr;
	m_wndProc = nullptr;
	ZeroMemory(&m_wndClassEx, sizeof(WNDCLASS));
	m_hWnd = nullptr;
	FillFeatureLevelArray();

	// Timing
	if (QueryPerformanceFrequency(&m_clockFrequency) == TRUE)
	{
		m_clockFrequencyObtained = true;
	}
	else
	{
		m_clockFrequencyObtained = false;
		ref->client->Sys_Error(ERR_FATAL, "Couldn't obtain clock frequency.");
	}

	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_12_1;

	m_d3dDevice = nullptr;
	m_d3dDevice1 = nullptr;
	m_immediateContext = nullptr;
	m_immediateContext1 = nullptr;
	m_swapChain = nullptr;
	m_swapChain1 = nullptr;
	m_backBufferRTV = nullptr;

	m_overlaySystem = std::make_unique<Subsystem2D>();

	m_d3dInitialized = false;
}

void dx11::System::BeginRegistration()
{
	//BOOST_LOG_NAMED_SCOPE("System::BeginRegistration");
	LOG_FUNC();

	if (!m_inRegistration)
	{
		BeginUpload();

		m_inRegistration = true;
	}
}

void dx11::System::EndRegistration()
{
	//BOOST_LOG_NAMED_SCOPE("System::EndRegistration");
	LOG_FUNC();

	if (m_inRegistration)
	{
		m_inRegistration = false;

		EndUpload();
	}
}

void dx11::System::BeginUpload()
{
	//BOOST_LOG_NAMED_SCOPE("System::BeginUpload");
	LOG_FUNC();

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
	//BOOST_LOG_NAMED_SCOPE("System::EndUpload");
	LOG_FUNC();

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
	//BOOST_LOG_NAMED_SCOPE("System::BeginFrame");
	LOG_FUNC();
	
	// Timing
	if ((m_clockFrequencyObtained) && (QueryPerformanceCounter(&m_clockFrameStart) == TRUE))
	{
		m_clockRunning = true;
	}
	else
	{
		m_clockRunning = false;
	}

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

	if (m_overlaySystem)
	{
		m_overlaySystem->Clear();
	}

	// Clear 3D deferred
	/*if (m_3DdeferredContext)
	{

	}*/
}

void dx11::System::RenderFrame(refdef_t * fd)
{
	//BOOST_LOG_NAMED_SCOPE("System::RenderFrame");
	LOG_FUNC();

	// Draw 3D
	//m_immediateContext->OMSetDepthStencilState(m_depthStencilState, 1);

	if ((fd == NULL) || (fd == nullptr))
	{
		LOG(warning) << "NULL refdef provided";
	}

	// Draw 2D
	//LOG(trace) << "Drawing 2D"
}

void dx11::System::EndFrame(void)
{
	//BOOST_LOG_NAMED_SCOPE("System::EndFrame");
	LOG_FUNC();
	
	if (m_overlaySystem)
	{
		// Draw 2D
		m_overlaySystem->Render();
	}

	if (m_swapChain)
	{
		// Switch the back buffer and the front buffer
		m_swapChain->Present(0, 0);
	}

	// Timing
	if ((m_clockRunning) && (QueryPerformanceCounter(&m_clockFrameEndCurrent) == TRUE))
	{
		m_frameTime = static_cast<double>(m_clockFrameEndCurrent.QuadPart - m_clockFrameStart.QuadPart) / m_clockFrequency.QuadPart;

		m_frameTimeEMA = EMA_ALPHA * m_frameTimeEMA + (1.0 - EMA_ALPHA) * m_frameTime;

		m_frameRateEMA = EMA_ALPHA * m_frameRateEMA + (1.0 - EMA_ALPHA) * (m_clockFrameEndCurrent.QuadPart - m_clockFrameEndPrevious.QuadPart);
		m_clockFrameEndPrevious = m_clockFrameEndCurrent;

		//LOG(trace) << "Frame <rate> " << m_frameRateEMA << " fps <time> " << (m_frameTime * 1000) << " ms";
	}

	m_clockRunning = false;
}

void dx11::System::D3D_Strings_f()
{
	LOG_FUNC();

	if ((m_adapterDesc.VendorId == 0x1414) && (m_adapterDesc.DeviceId == 0x8c))
	{
		// Microsoft Basic Render Driver
		ref->client->Con_Printf(PRINT_ALL, "WARNING: Microsoft Basic Render Driver is active.\n Performance of this application may be unsatisfactory.\n Please ensure that your video card is Direct3D10/11 capable\n and has the appropriate driver installed.");
	}

	ref->client->Con_Printf(PRINT_ALL, "D3D Feature Level: " + StringForFeatureLevel(m_featureLevel));

	// We need this to get a compliant string
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertToUTF8;

	std::stringstream hexValue;

	ref->client->Con_Printf(PRINT_ALL, "Adapter Description: " + convertToUTF8.to_bytes(m_adapterDesc.Description));
	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.VendorId;
	ref->client->Con_Printf(PRINT_ALL, "        Vendor ID: " + hexValue.str());
	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.DeviceId;
	ref->client->Con_Printf(PRINT_ALL, "        Device ID: " + hexValue.str());
	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.SubSysId;
	ref->client->Con_Printf(PRINT_ALL, "        Subsystem ID: " + hexValue.str());
	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.Revision;
	ref->client->Con_Printf(PRINT_ALL, "        Revision: " + hexValue.str());
	ref->client->Con_Printf(PRINT_ALL, "Dedicated Video Memory: " + std::to_string(m_adapterDesc.DedicatedVideoMemory));
	ref->client->Con_Printf(PRINT_ALL, "Dedicated System Memory: " + std::to_string(m_adapterDesc.DedicatedSystemMemory));
	ref->client->Con_Printf(PRINT_ALL, "Shared System Memory: " + std::to_string(m_adapterDesc.SharedSystemMemory));
}

extern "C" __declspec(dllexport) void SHIM_D3D_Strings_f (void)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->D3D_Strings_f();
	}
}


/*
** VID_CreateWindow
*/
bool dx11::System::VID_CreateWindow()
{
	//BOOST_LOG_NAMED_SCOPE("System::VID_CreateWindow");
	LOG_FUNC();

	RECT				r			= {};
	ZeroMemory(&r, sizeof(RECT));
	DWORD				stylebits	= 0;
	int					x			= 0,
						y			= 0,
						w			= 0,
						h			= 0;
	int					exstyle		= 0;
	const LONG			width		= ref->cvars->r_customWidth->Int();
	const LONG			height		= ref->cvars->r_customHeight->Int();
	const bool			fullscreen	= ref->cvars->vid_fullscreen->Bool();

	ref->client->Con_Printf(PRINT_ALL, "Creating window");

	/* Register the frame class */
	m_wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	m_wndClassEx.lpfnWndProc = m_wndProc;
	m_wndClassEx.cbClsExtra = 0;
	m_wndClassEx.cbWndExtra = 0;
	m_wndClassEx.hInstance = m_hInstance;
	m_wndClassEx.hIcon = (HICON)LoadImage(m_hInstance, "q2.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	m_wndClassEx.hIconSm = (HICON)LoadImage(m_hInstance, "q2.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	m_wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	//m_wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_GRAYTEXT + 1);
	m_wndClassEx.lpszMenuName = nullptr;
	m_wndClassEx.lpszClassName = WINDOW_CLASS_NAME;
	m_wndClassEx.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&m_wndClassEx))
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't register window class");
	}

	// Note that adding the sysmenu and hitting close puts the game thread into an infinite loop, so don't do it
	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP | WS_VISIBLE; //| WS_SYSMENU;
	}
	else
	{
		exstyle = 0;
		stylebits = WINDOW_STYLE; //| WS_SYSMENU;
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
		x = ref->cvars->vid_xPos->Int();
		y = ref->cvars->vid_yPos->Int();
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
	//BOOST_LOG_NAMED_SCOPE("System::VID_DestroyWindow");
	LOG_FUNC();
	
	if (m_hWnd != nullptr)
	{
		ref->client->Con_Printf(PRINT_ALL, "...destroying window\n");

		ShowWindow(m_hWnd, SW_SHOWNORMAL);	// prevents leaving empty slots in the taskbar
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		UnregisterClass(WINDOW_CLASS_NAME, m_hInstance);
	}
}


bool dx11::System::Initialize(HINSTANCE hInstance, WNDPROC wndProc)
{
	//BOOST_LOG_NAMED_SCOPE("System::Initialize");
	LOG_FUNC();

	if (ref->client != nullptr)
	{
		ref->client->Con_Printf(PRINT_ALL, "ref_dx11 version: " REF_VERSION "\n");
	}

	LOG(info) << "Starting up.";

	Shutdown();

	m_hInstance = hInstance;
	m_wndProc = wndProc;

	if (!VID_CreateWindow())
	{
		LOG(error) << "Failed to create window";
		return false;
	}

	if (!D3D_InitDevice())
	{
		LOG(error) << "Failed to create D3D device";
		return false;
	}

	if ((!m_overlaySystem) || (!m_overlaySystem->Initialize()))
	{
		LOG(error) << "Failed to create 2D overlay system (GUI)";
		return false;
	}

	return true;
}

void dx11::System::Shutdown()
{
	//BOOST_LOG_NAMED_SCOPE("System::Shutdown");
	LOG_FUNC();

	if (m_overlaySystem)
	{
		m_overlaySystem->Shutdown();
	}

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
	//BOOST_LOG_NAMED_SCOPE("System::AppActivate");
	LOG_FUNC();

	if (active)
	{
		LOG(info) << "Restoring Window.";

		SetForegroundWindow(m_hWnd);
		ShowWindow(m_hWnd, SW_RESTORE);
	}
	else
	{
		if (ref->cvars->vid_fullscreen->Bool())
		{
			LOG(info) << "Minimizing Window.";

			ShowWindow(m_hWnd, SW_MINIMIZE);
		}
	}
}

bool dx11::System::D3D_InitDevice()
{
	//BOOST_LOG_NAMED_SCOPE("System::D3D_InitDevice");
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	RECT rc = {};
	UINT createDeviceFlags = 0;

	GetClientRect(m_hWnd, &rc);
	m_windowWidth = msl::utilities::SafeInt<unsigned int>(rc.right - rc.left);
	m_windowHeight = msl::utilities::SafeInt<unsigned int>(rc.bottom - rc.top);

	LOG(info) << "Creating D3D Device.";

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

	UINT numFeatureLevels = ARRAYSIZE(m_featureLevelArray);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, m_featureLevelArray, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);

		if (hr == E_INVALIDARG)
		{
			LOG(warning) << "DirectX 11.1 runtime will not recognize D3D_FEATURE_LEVEL_12_x, so trying again without them.";
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &m_featureLevelArray[2], numFeatureLevels - 2, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);

			if (hr == E_INVALIDARG)
			{
				LOG(warning) << "DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1+ so trying again without them.";
				hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &m_featureLevelArray[3], numFeatureLevels - 3, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);
			}
		}

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create D3D device.";
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
				
				ZeroMemory(&m_adapterDesc, sizeof(DXGI_ADAPTER_DESC));
				hr = adapter->GetDesc(&m_adapterDesc);

				if (SUCCEEDED(hr))
				{
					D3D_Strings_f();

					// Create command
					ref->client->Cmd_AddCommand("dx11_strings", SHIM_D3D_Strings_f);
				}

				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	if (FAILED(hr))
	{
		LOG(error) << "Unable to obtain DGXIFactory.";
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
		LOG(error) << "Unable to obtain DGXIFactory.";
		return false;
	}

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

	if (FAILED(hr))
	{
		LOG(error) << "Unable to get BackBuffer.";
		return false;
	}

	hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_backBufferRTV);

	pBackBuffer->Release();

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create BackBuffer RenderTargetView.";
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

	m_d3dInitialized = true;

	return m_d3dInitialized;
}



void dx11::System::D3D_Shutdown()
{
	//BOOST_LOG_NAMED_SCOPE("System::D3D_Shutdown");
	LOG_FUNC();

	LOG(info) << "Shutting down D3D.";

	if (m_immediateContext)
	{
		m_immediateContext->ClearState();
	}

	if (m_swapChain)
	{
		LOG(info) << "Switching to windowed mode to allow proper cleanup.";
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

	if (m_overlaySystem)
	{
		m_overlaySystem->Shutdown();
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