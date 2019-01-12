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
ref_dx12
2019 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

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

void dx12::Dx::FillFeatureLevelArray(void)
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

dx12::Dx::Dx()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	FillFeatureLevelArray();

	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_12_1;

/*	m_d2dFactory = nullptr;
	m_d2dDevice = nullptr;
	m_d2dContext = nullptr;
	m_d2dCommandList = nullptr;
	*/
	m_d3dDevice = nullptr;
	m_d3dDevice1 = nullptr;
	//m_immediateContext = nullptr;
	//m_immediateContext1 = nullptr;
	m_swapChain = nullptr;
	m_swapChain1 = nullptr;
	//m_backBufferRTV = nullptr;

	subsystem3D = std::make_unique<Subsystem3D>();
	subsystem2D = std::make_unique<Subsystem2D>();
	subsystemText = std::make_unique<SubsystemText>();

	m_d3dInitialized = false;
}

void dx12::Dx::BeginFrame(void)
{
	LOG_FUNC();

	// Timing
	if ((ref->sys->m_clockFrequencyObtained) && (QueryPerformanceCounter(&m_clockFrameStart) == TRUE))
	{
		m_clockRunning = true;
	}
	else
	{
		m_clockRunning = false;
	}
	/*
	// Clear immediate context
	if (m_immediateContext)
	{
		if (m_backBufferRTV)
		{
#ifndef _DEBUG
			// clear the back buffer to black
			m_immediateContext->ClearRenderTargetView(m_backBufferRTV, DirectX::Colors::Black);
#else
			// clear the back buffer to a deep blue
			m_immediateContext->ClearRenderTargetView(m_backBufferRTV, DirectX::Colors::Blue);
#endif
		}

		if (m_depthStencilView)
		{
			// Clear the depth buffer
			m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D12_CLEAR_DEPTH | D3D12_CLEAR_STENCIL, 1.0f, 0);
		}
	}
	*/
	if (subsystem2D)
	{
		subsystem2D->Update();
		subsystem2D->Clear();
	}

	if (subsystem3D)
	{
		subsystem3D->Clear();
	}

#if defined(DEBUG) || defined (_DEBUG)
	DumpD3DDebugMessagesToLog();
#endif
}

void dx12::Dx::RenderFrame(refdef_t * fd)
{
	LOG_FUNC();

	// Draw 3D
	//m_immediateContext->OMSetDepthStencilState(m_depthStencilState, 1);

	if ((fd == NULL) || (fd == nullptr))
	{
		LOG(warning) << "NULL refdef provided";
	}

	// Z-Prepass


}

void dx12::Dx::EndFrame(void)
{
	LOG_FUNC();

	if (subsystem2D)
	{
		// Draw 2D
		subsystem2D->Render();
	}
/*
	// Clear the PS binding
	ID3D12ShaderResourceView* clearSRV = { NULL };
	ref->sys->dx->m_immediateContext->PSSetShaderResources(0, 1, &clearSRV);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_immediateContext->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
*/
	if (m_swapChain)
	{
		// Switch the back buffer and the front buffer
		m_swapChain->Present(ref->cvars->Vsync->UInt(), 0);
	}
	/*
	// Set the overlay RTV as the current render target
	ref->sys->dx->subsystem2D->m_2DdeferredContext->OMSetRenderTargets(1, &ref->sys->dx->subsystem2D->m_2DoverlayRTV, ref->sys->dx->m_depthStencilView);
	*/
	// Timing
	if ((m_clockRunning) && (QueryPerformanceCounter(&m_clockFrameEndCurrent) == TRUE))
	{
		m_frameTime = static_cast<double>(m_clockFrameEndCurrent.QuadPart - m_clockFrameStart.QuadPart) / ref->sys->m_clockFrequency.QuadPart;

		m_frameTimeEMA = EMA_ALPHA * m_frameTimeEMA + (1.0 - EMA_ALPHA) * m_frameTime;

		m_frameRateEMA = EMA_ALPHA * m_frameRateEMA + (1.0 - EMA_ALPHA) * (m_clockFrameEndCurrent.QuadPart - m_clockFrameEndPrevious.QuadPart);
		m_clockFrameEndPrevious = m_clockFrameEndCurrent;

		//LOG(trace) << "Frame <rate> " << m_frameRateEMA << " fps <time> " << (m_frameTime * 1000) << " ms";
	}

	m_clockRunning = false;

#if defined(DEBUG) || defined (_DEBUG)
	DumpD3DDebugMessagesToLog();
#endif
}

void dx12::Dx::D3D_Strings_f()
{
	LOG_FUNC();

	if ((m_adapterDesc.VendorId == 0x1414) && (m_adapterDesc.DeviceId == 0x8c))
	{
		// Microsoft Basic Render Driver
		ref->client->Con_Printf(PRINT_ALL, "WARNING: Microsoft Basic Render Driver is active.\n Performance of this application may be unsatisfactory.\n Please ensure that your video card is Direct3D10/11 capable\n and has the appropriate driver installed.");
	}

	ref->client->Con_Printf(PRINT_ALL, "D3D Feature Level: " + StringForFeatureLevel(m_featureLevel));

	std::stringstream hexValue;

	ref->client->Con_Printf(PRINT_ALL, "    Adapter Description: " + ref->sys->ToString(m_adapterDesc.Description));
	ref->client->Con_Printf(PRINT_ALL, "");

	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.VendorId;
	ref->client->Con_Printf(PRINT_ALL, "              Vendor ID: " + hexValue.str());

	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.DeviceId;
	ref->client->Con_Printf(PRINT_ALL, "              Device ID: " + hexValue.str());

	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.SubSysId;
	ref->client->Con_Printf(PRINT_ALL, "           Subsystem ID: " + hexValue.str());

	hexValue.str(std::string());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.Revision;
	ref->client->Con_Printf(PRINT_ALL, "               Revision: " + hexValue.str());

	ref->client->Con_Printf(PRINT_ALL, " Dedicated Video Memory: " + std::to_string(m_adapterDesc.DedicatedVideoMemory));
	ref->client->Con_Printf(PRINT_ALL, "Dedicated System Memory: " + std::to_string(m_adapterDesc.DedicatedSystemMemory));
	ref->client->Con_Printf(PRINT_ALL, "   Shared System Memory: " + std::to_string(m_adapterDesc.SharedSystemMemory));
}

extern "C" __declspec(dllexport) void SHIM_D3D_Strings_f(void)
{
	if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr) && (dx12::ref->sys->dx != nullptr))
	{
		dx12::ref->sys->dx->D3D_Strings_f();
	}
}

bool dx12::Dx::Initialize(HWND hWnd)
{
	LOG_FUNC();

	LOG(info) << "Starting up.";

	Shutdown();

	if (!InitDevice(hWnd))
	{
		LOG(error) << "Failed to create D3D device";
		return false;
	}

	if ((!subsystem2D) || (!subsystem2D->Initialize()))
	{
		LOG(error) << "Failed to create 2D overlay subsystem (GUI)";
		return false;
	}

	if ((!subsystemText) || (!subsystemText->Initialize()))
	{
		LOG(error) << "Failed to create text subsystem.";
		return false;
	}

	if ((!subsystem3D) || (!subsystem3D->Initialize()))
	{
		LOG(error) << "Failed to create 3D subsystem";
		return false;
	}

	return true;
}

void dx12::Dx::Shutdown()
{
	LOG_FUNC();

	if (subsystem2D)
	{
		subsystem2D->Shutdown();
	}

	if (subsystem3D)
	{
		subsystem3D->Shutdown();
	}

	if (m_d3dInitialized)
	{
		D3D_Shutdown();
	}
}

bool dx12::Dx::GetAdapter(IDXGIFactory6* pFactory, IDXGIAdapter1** ppAdapter)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();

	return true;
}

bool dx12::Dx::InitDebug()
{
#if defined(DEBUG) || defined (_DEBUG)

	if (m_d3dDevice)
	{
		// Obtain global debug device
		if (SUCCEEDED(m_d3dDevice->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(&d3dDebug))))
		{
			LOG(info) << "DEBUG: Successfully created D3D Debug Device.";

			if (SUCCEEDED(d3dDebugDev->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&d3dInfoQueue)))
			{
				LOG(info) << "DEBUG: Successfully created D3D Debug Info Queue.";

				d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				//d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
				};

				D3D12_INFO_QUEUE_FILTER filter;
				ZeroMemory(&filter, sizeof(D3D12_INFO_QUEUE_FILTER));
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);

				return true;
			}
		}
	}
#endif
	return false;
}

bool dx12::Dx::InitDevice(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	RECT rc = {};
	UINT createDeviceFlags = 0; // D3D12_CREATE_DEVICE_BGRA_SUPPORT; // This flag adds support for surfaces with a different color channel ordering than the API default for compatibility with Direct2D.

	GetClientRect(hWnd, &rc);
	m_windowWidth = msl::utilities::SafeInt<unsigned int>(rc.right - rc.left);
	m_windowHeight = msl::utilities::SafeInt<unsigned int>(rc.bottom - rc.top);

	LOG(info) << "Creating DirectX devices.";

	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));

	if (FAILED(hr))
	{
		LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_1, so trying D3D_FEATURE_LEVEL_12_0.";
		hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));

		if (FAILED(hr))
		{
			LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_1, so creating WARP device.";

			ComPtr<IDXGIFactory6> dxgiFactory;
			CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

			ComPtr<IDXGIAdapter> pWarpAdapter;
			dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

			hr = D3D12CreateDevice(pWarpAdapter.Get(),	D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));
		}
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Created D3D12 device.";
		return true;
	}

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create D3D device.";
		return false;
	}

#ifdef _DEBUG
	InitDebug();
#endif

	InitFactory(hWnd);
	
	InitCommandObjects();

	InitSwapChain(hWnd);

	// Create a render target view
	ID3D12Resource* pBackBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&pBackBuffer));

	if (FAILED(hr))
	{
		LOG(error) << "Unable to get BackBuffer.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created BackBuffer.";
	}

	/*
	hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_backBufferRTV);
	*/

	pBackBuffer->Release();

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create BackBuffer RenderTargetView.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created BackBuffer RenderTargetView.";
	}

	LOG(info) << "Setting immediate context render target to BackBuffer RenderTargetView.";

	/*
	m_immediateContext->OMSetRenderTargets(1, &m_backBufferRTV, nullptr);
	*/

	// Setup the viewport
	D3D12_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D12_VIEWPORT));
	viewport.Width = (FLOAT)m_windowWidth;
	viewport.Height = (FLOAT)m_windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	LOG(info) << "Setting viewport.";

	/*
	m_immediateContext->RSSetViewports(1, &viewport);
	*/

	UINT viewportCount = 0;

	/*
	m_immediateContext->RSGetViewports(&viewportCount, nullptr);
	*/

	LOG(info) << std::to_string(viewportCount) << " viewports bound.";

	m_d3dInitialized = true;

	return m_d3dInitialized;
}

bool dx12::Dx::InitFactory(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		LOG(info) << "Creating DXGI Factory 6.";

		// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
		IDXGIDevice* dxgiDevice = nullptr;

		hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Successfully created DXGI device.";

			IDXGIAdapter* adapter = nullptr;

			hr = dxgiDevice->GetAdapter(&adapter);

			if (SUCCEEDED(hr))
			{
				LOG(info) << "Successfully obtained DXGI adapter.";

				ZeroMemory(&m_adapterDesc, sizeof(DXGI_ADAPTER_DESC));
				hr = adapter->GetDesc(&m_adapterDesc);

				if (SUCCEEDED(hr))
				{
					D3D_Strings_f();

					// Create command
					ref->client->Cmd_AddCommand("dx12_strings", SHIM_D3D_Strings_f);
				}

				hr = adapter->GetParent(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&m_dxgiFactory));
				if (SUCCEEDED(hr))
				{
					LOG(info) << "Successfully obtained DXGI Factory 6.";

					// Block the ALT+ENTER shortcut
					m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
				}

				adapter->Release();
			}

			dxgiDevice->Release();

			return true;
		}
		else if (FAILED(hr))
		{
			LOG(error) << "Unable to create DGXI Factory.";
		}
	}

	return false;
}

bool dx12::Dx::InitCommandObjects()
{
	LOG_FUNC();

	return false;
}

bool dx12::Dx::InitSwapChain(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_dxgiFactory)
	{
		// Create swap chain
		LOG(info) << "Successfully obtained DXGI Factory2. System is DirectX 11.1 or greater.";

		// DirectX 11.1 or later
		hr = m_d3dDevice->QueryInterface(__uuidof(ID3D12Device1), reinterpret_cast<void**>(&m_d3dDevice1));

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Successfully obtained D3D Device1. Obtaining immediate DeviceContext1.";

			/*
			(void)m_immediateContext->QueryInterface(__uuidof(ID3D12DeviceContext1), reinterpret_cast<void**>(&m_immediateContext1));
			*/
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
		swapChainDesc.Width = m_windowWidth;
		swapChainDesc.Height = m_windowHeight;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;			// BGRA for D2D support (also 5% faster, supposedly)
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Stereo = FALSE;

		hr = m_dxgiFactory->CreateSwapChainForHwnd(m_d3dDevice, hWnd, &swapChainDesc, nullptr, nullptr, &m_swapChain1);

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Successfully created SwapChain1. Obtaining SwapChain.";

			hr = m_swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_swapChain));

			return true;
		}
	}
	else
	{
		LOG(warning) << "Failed to obtain DXGI Factory2. System is DirectX 11 or less.";

		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = m_windowWidth;
		swapChainDesc.BufferDesc.Height = m_windowHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;			// BGRA for D2D support (also 5% faster, supposedly)
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = m_dxgiFactory->CreateSwapChain(m_d3dDevice, &swapChainDesc, &m_swapChain);

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Successfully created SwapChain.";

			return true;
		}
		else
		{
			LOG(error) << "Unable to create SwapChain.";
		}
	}

	return false;
}

void dx12::Dx::D3D_Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down D3D.";

	if (m_swapChain)
	{
		LOG(info) << "Switching to windowed mode to allow proper cleanup.";
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}

	/*
	SAFE_RELEASE(m_backBufferRTV);

	SAFE_RELEASE(m_depthStencilState);

	SAFE_RELEASE(m_depthStencilView);
	*/

	SAFE_RELEASE(m_swapChain1);

	SAFE_RELEASE(m_swapChain);

	if (subsystemText)
	{
		subsystemText->Shutdown();
	}

	if (subsystem2D)
	{
		subsystem2D->Shutdown();
	}

	if (subsystem3D)
	{
		subsystem3D->Shutdown();
	}

/*	SAFE_RELEASE(m_d2dContext);

	SAFE_RELEASE(m_d2dDevice);

	SAFE_RELEASE(m_d2dFactory);*/

	/*
	if (m_immediateContext)
	{
		// https://blogs.msdn.microsoft.com/chuckw/2012/11/30/direct3d-sdk-debug-layer-tricks/
		// It can also help to call ClearState and then Flush on the immediate context just before doing the report to ensure nothing is being kept alive by being bound to the render pipeline or because of lazy destruction.
		m_immediateContext->ClearState();
		m_immediateContext->Flush();

		if (m_immediateContext1)
		{
			m_immediateContext1->ClearState();
			m_immediateContext1->Flush();

			SAFE_RELEASE(m_immediateContext1);
		}

		SAFE_RELEASE(m_immediateContext);
	}
	*/

	SAFE_RELEASE(m_d3dDevice1);

	SAFE_RELEASE(m_d3dDevice);

	m_d3dInitialized = false;
}