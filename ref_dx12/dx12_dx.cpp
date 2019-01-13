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

	m_dxgiFactory = nullptr;
	m_dxgiAdapter = nullptr;

	m_d3dDevice = nullptr;

	m_fence = nullptr;
	m_currentFence = 0;

	m_commandQueue = nullptr;
	m_directCmdListAlloc = nullptr;
	m_commandListGfx = nullptr;

	m_swapChain = nullptr;

	m_descriptorHeap = nullptr;
	m_descriptorsAllocated = 0;
	m_descriptorSizeRTV = 0;
	m_descriptorSizeDSV = 0;
	m_descriptorSizeCBVSRVUAV = 0;

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

	LOG(info) << "Starting up...";

	// Make sure everything is clear
	LOG(info) << "Preemptively shutdown any running subsystems.";
	Shutdown();

	// Get window dimensions
	RECT rc = {};
	GetClientRect(hWnd, &rc);
	m_windowWidth = msl::utilities::SafeInt<unsigned int>(rc.right - rc.left);
	m_windowHeight = msl::utilities::SafeInt<unsigned int>(rc.bottom - rc.top);

	if (!InitFactory(hWnd))
	{
		LOG(error) << "Failed to create DXGI factory.";
		return false;
	}

	if (!InitAdapter())
	{
		LOG(error) << "Failed to create DXGI adapter.";
		return false;
	}

	if (!InitDevice(hWnd))
	{
		LOG(error) << "Failed to create D3D device.";
		return false;
	}

	if (!InitFences())
	{
		LOG(error) << "Failed to create fences.";
		return false;
	}

	if (!InitCommandObjects())
	{
		LOG(error) << "Failed to create command objects.";
		return false;
	}

	if (!InitSwapChain(hWnd))
	{
		LOG(error) << "Failed to create Swap Chain.";
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

bool dx12::Dx::InitFactory(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	LOG(info) << "Creating DXGI factory...";

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created DXGI factory.";

		// Block the ALT+ENTER shortcut
		m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		return true;
	}

	LOG(error) << "Unable to create DGXI factory.";

	return false;
}

bool dx12::Dx::InitAdapter()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_dxgiFactory)
	{
		LOG(info) << "Obtaining DXGI adapter...";

		ComPtr<IDXGIAdapter4> adapter;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter4), reinterpret_cast<void**>(adapter.ReleaseAndGetAddressOf())); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		m_dxgiAdapter = adapter.Detach();

		LOG(info) << "Successfully obtained DXGI adapter.";

		ZeroMemory(&m_adapterDesc, sizeof(DXGI_ADAPTER_DESC));
		hr = m_dxgiAdapter->GetDesc(&m_adapterDesc);

		if (SUCCEEDED(hr))
		{
			D3D_Strings_f();

			// Create command
			ref->client->Cmd_AddCommand("dx12_strings", SHIM_D3D_Strings_f);
		}
	}

	if (SUCCEEDED(hr))
	{
		return true;
	}

	LOG(error) << "Unable to create DGXI adapter.";

	return false;
}

bool dx12::Dx::InitDeviceDebug()
{
#if defined(DEBUG) || defined (_DEBUG)

	if (m_d3dDevice)
	{
		// Obtain global debug device
		if (SUCCEEDED(m_d3dDevice->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(&d3dDebugDev))))
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

	LOG(error) << "Unable to create D3D device debug objects.";
#endif
	return false;
}

bool dx12::Dx::InitDevice(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	LOG(info) << "Creating DirectX 12 device...";

	hr = D3D12CreateDevice(m_dxgiAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));

	if (FAILED(hr))
	{
		LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_1, so trying D3D_FEATURE_LEVEL_12_0.";
		hr = D3D12CreateDevice(m_dxgiAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice));

		if (FAILED(hr))
		{
			LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_1, so attempt to create WARP device.";

			ComPtr<IDXGIFactory6> dxgiFactory;
			CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

			ComPtr<IDXGIAdapter> pWarpAdapter;
			dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

			hr = D3D12CreateDevice(pWarpAdapter.Get(),	D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));
			if (FAILED(hr))
			{
				LOG(warning) << "Failed to create WARP device!";
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created DirectX 12 device.";
	}
	else
	{
		LOG(error) << "Unable to create D3D12 device.";
		return false;
	}

#ifdef _DEBUG
	InitDeviceDebug();
#endif

#if 0
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

#endif

	m_d3dInitialized = true;

	return m_d3dInitialized;
}

bool dx12::Dx::InitFences()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		LOG(info) << "Creating fences...";

		hr = m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_fence));
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created fence.";

		return true;
	}
	
	LOG(error) << "Failed to create fences.";

	return false;
}

bool dx12::Dx::InitCommandObjects()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		// Create swap chain
		LOG(info) << "Creating command objects...";

		LOG(info) << "Creating Command Queue";
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		hr = m_d3dDevice->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&m_commandQueue));

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Creating Command Allocator";

			hr = m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdListAlloc));

			if (SUCCEEDED(hr))
			{
				LOG(info) << "Creating Graphics Command List";

				hr = m_d3dDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					m_directCmdListAlloc, // Associated command allocator
					nullptr,                   // Initial PipelineStateObject
					__uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_commandListGfx));

				if (SUCCEEDED(hr))
				{
					LOG(info) << "Closing Graphics Command List";

					// Start off in a closed state.  This is because the first time we refer 
					// to the command list we will Reset it, and it needs to be closed before
					// calling Reset.
					m_commandListGfx->Close();
				}
				else
				{
					LOG(error) << "Failed to create Graphics Command List.";
				}
			}
			else
			{
				LOG(error) << "Failed to create Command Allocator.";
			}
		}
		else
		{
			LOG(error) << "Failed to create Command Queue.";
		}
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created command objects.";

		return true;
	}

	LOG(error) << "Failed to create command objects.";

	return false;
}

bool dx12::Dx::InitSwapChain(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_dxgiFactory)
	{
		// Create swap chain
		LOG(info) << "Creating swap chain...";

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
		swapChainDesc.BufferCount = ref->cvars->bufferCount->Int();
		swapChainDesc.Width = m_windowWidth;
		swapChainDesc.Height = m_windowHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = ref->cvars->samplesPerPixel->Int();

		IDXGISwapChain1* pSwapChain = nullptr;
		hr = m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue, hWnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);

		if (SUCCEEDED(hr))
		{
			hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));
		}
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created Swap Chain.";

		return true;
	}

	LOG(error) << "Unable to create Swap Chain.";

	return false;
}

bool dx12::Dx::InitDescriptorHeaps()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		// Get descriptor sizes
		LOG(info) << "Obtaining descriptor heap element sizes...";
		m_descriptorSizeRTV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_RTV: " << m_descriptorSizeRTV;

		m_descriptorSizeDSV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_DSV: " << m_descriptorSizeDSV;

		m_descriptorSizeCBVSRVUAV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: " << m_descriptorSizeCBVSRVUAV;

		// Create descriptor heaps
		LOG(info) << "Creating descriptor heaps...";

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

		LOG(info) << "Allocating heap for 3 descriptors:";
		LOG(info) << " 2 - bottom and top level acceleration structures";
		LOG(info) << " 1 - raytracing output texture SRV";

		descriptorHeapDesc.NumDescriptors = 3;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;

		hr = m_d3dDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created descriptor heap.";

		return true;
	}

	LOG(error) << "Unable to create descriptor heap.";

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

	SAFE_RELEASE(m_descriptorHeap);

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

	SAFE_RELEASE(m_commandListGfx);

	SAFE_RELEASE(m_directCmdListAlloc);

	SAFE_RELEASE(m_commandQueue);

	SAFE_RELEASE(m_fence);

	SAFE_RELEASE(m_d3dDevice);

	SAFE_RELEASE(m_dxgiAdapter);

	SAFE_RELEASE(m_dxgiFactory);

	m_d3dInitialized = false;
}