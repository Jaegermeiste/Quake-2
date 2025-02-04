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

static D3D_FEATURE_LEVEL FeatureLevelForString(std::wstring featureLevelString)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_2;

	std::map<std::wstring, D3D_FEATURE_LEVEL> featureLevelMap;
	featureLevelMap[STR(D3D_FEATURE_LEVEL_12_2)] = D3D_FEATURE_LEVEL_12_2;
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

static std::wstring StringForFeatureLevel(D3D_FEATURE_LEVEL  featureLevel)
{
	std::wstring featureLevelString = L"D3D_FEATURE_LEVEL_12_2";

	std::map<D3D_FEATURE_LEVEL, std::wstring> featureLevelMap;
	featureLevelMap[D3D_FEATURE_LEVEL_12_2] = STR(D3D_FEATURE_LEVEL_12_2);
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
	m_featureLevelArray[0] = D3D_FEATURE_LEVEL_12_2;
	m_featureLevelArray[1] = D3D_FEATURE_LEVEL_12_1;
	m_featureLevelArray[2] = D3D_FEATURE_LEVEL_12_0;
	m_featureLevelArray[3] = D3D_FEATURE_LEVEL_11_1;
	m_featureLevelArray[4] = D3D_FEATURE_LEVEL_11_0;
	m_featureLevelArray[5] = D3D_FEATURE_LEVEL_10_1;
	m_featureLevelArray[6] = D3D_FEATURE_LEVEL_10_0;
	m_featureLevelArray[7] = D3D_FEATURE_LEVEL_9_3;
	m_featureLevelArray[8] = D3D_FEATURE_LEVEL_9_2;
	m_featureLevelArray[9] = D3D_FEATURE_LEVEL_9_1;
}

dx12::Dx::Dx()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	FillFeatureLevelArray();

	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = FeatureLevelForString(ref->cvars->featureLevel->String());

	m_dxgiFactory = nullptr;
	m_dxgiAdapter = nullptr;

	m_d3dDevice = nullptr;

	m_fence = nullptr;
	m_fenceEvent = nullptr;
	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		m_fenceValues[i] = 0;
	}

	m_commandQueue = nullptr;
	m_commandListSwap = nullptr;

	m_multisampleCount = ref->cvars->samplesPerPixel->UInt();
	m_swapChain = nullptr;

	m_descriptorHeapRTV = nullptr;
	m_descriptorHeapDSV = nullptr;
	m_descriptorHeapCBVSRVUAV = nullptr;

	m_backBufferCount = ref->cvars->bufferCount->UInt();
	if (m_backBufferCount < MIN_BACK_BUFFERS)
	{
		m_backBufferCount = MIN_BACK_BUFFERS;
		ref->cvars->bufferCount->Set(MIN_BACK_BUFFERS);
	}
	else if (m_backBufferCount > MAX_BACK_BUFFERS)
	{
		m_backBufferCount = MAX_BACK_BUFFERS;
		ref->cvars->bufferCount->Set(MAX_BACK_BUFFERS);
	}
	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		m_backBufferRenderTargets[i] = nullptr;
		m_backBufferRenderTargetStates[i] = D3D12_RESOURCE_STATE_COMMON;
	}
	m_backBufferIndex = 0;

	ZeroMemory(&m_viewport, sizeof(D3D12_VIEWPORT));
	ZeroMemory(&m_scissorRect, sizeof(D3D12_RECT));

	subsystem3D = std::make_unique<Subsystem3D>();
	subsystem2D = std::make_unique<Subsystem2D>();
	subsystemText = std::make_unique<SubsystemText>();

	m_d3dInitialized = false;
}

void dx12::Dx::BeginFrame(void)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	// Timing
	if ((ref->sys->m_clockFrequencyObtained) && (QueryPerformanceCounter(&m_clockFrameStart) == TRUE))
	{
		m_clockRunning = true;
	}
	else
	{
		m_clockRunning = false;
	}

	if (m_commandListSwap)
	{
		m_commandListSwap->Prepare();

		if (m_backBufferRenderTargetStates[m_backBufferIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET)
		{
			// Indicate that the back buffer will be used as a render target.
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTargets[m_backBufferIndex].Get(), m_backBufferRenderTargetStates[m_backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_commandListSwap->List()->ResourceBarrier(1, &barrier);
			m_backBufferRenderTargetStates[m_backBufferIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}

		// Set the render target
		auto rtvHandle = m_descriptorHeapRTV->GetCPUDescriptorHandle(m_backBufferRTVHandles[m_backBufferIndex]);
		m_commandListSwap->List()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		m_commandListSwap->SetPipelineState(m_pipelineState.Get());
	}

	if (m_commandListSwap && m_commandListSwap->IsOpen()) {

		if (ref->cvars->clear->Bool())
		{
			float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

#if defined(DEBUG) || defined (_DEBUG)			
			// Visualize back buffer flip
			if (m_backBufferIndex % 2)
			{
				clearColor[0] = 0.4f;
				clearColor[2] = 0.0f;
			}
#endif			

			m_commandListSwap->List()->ClearRenderTargetView(m_descriptorHeapRTV->GetCPUDescriptorHandle(m_backBufferRTVHandles[m_backBufferIndex]), clearColor, 0, nullptr);
		}
	}

	if (subsystem2D)
	{
		subsystem2D->Update();
		subsystem2D->Clear();

		ref->draw->BeginFrame();
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

	HRESULT hr = E_UNEXPECTED;

	if (subsystem2D)
	{
		ref->draw->EndFrame();

		// Draw 2D
		subsystem2D->Render();
	}

	if (m_commandQueue && m_commandListSwap)
	{
		if (m_backBufferRenderTargetStates[m_backBufferIndex] != D3D12_RESOURCE_STATE_PRESENT)
		{
			// Indicate that the back buffer will now be used to present.
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTargets[m_backBufferIndex].Get(), m_backBufferRenderTargetStates[m_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT);
			m_commandListSwap->List()->ResourceBarrier(1, &barrier);
			m_backBufferRenderTargetStates[m_backBufferIndex] = D3D12_RESOURCE_STATE_PRESENT;
		}

		m_commandListSwap->Close();
		m_commandListSwap->Execute();
	}

	if (m_swapChain)
	{
		// Switch the back buffer and the front buffer
		m_swapChain->Present(ref->cvars->Vsync->UInt(), 0);

		WaitForGPU();
		
		m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	// Timing
	if ((m_clockRunning) && (QueryPerformanceCounter(&m_clockFrameEndCurrent) == TRUE))
	{
		m_frameTime = static_cast<double>(m_clockFrameEndCurrent.QuadPart - m_clockFrameStart.QuadPart) / ref->sys->m_clockFrequency.QuadPart;

		m_frameTimeEMA = EMA_ALPHA * m_frameTimeEMA + (1.0 - EMA_ALPHA) * m_frameTime;

		m_frameRateEMA = EMA_ALPHA * m_frameRateEMA + (1.0 - EMA_ALPHA) * (1 / (m_frameTime + 0.000000000001));
		m_clockFrameEndPrevious = m_clockFrameEndCurrent;

		//LOG(trace) << "Frame <rate> " << m_frameRateEMA << " fps <time> " << m_frameTime << " ms";
#if defined(DEBUG) || defined (_DEBUG)
		// https://github.com/d3dcoder/d3d12book/blob/master/Common/d3dApp.cpp
		static unsigned int frameCount = 0;
		static double timeElapsed = 0.0f;
		static double totalTime = 0.0f;
		static unsigned int fps = 0;

		frameCount++;
		totalTime += m_frameTime;

		// Compute averages over one second period.
		if ((totalTime - timeElapsed) >= 1.0f)
		{
			fps = frameCount; // fps = frameCnt / 1

			// Reset for next average.
			frameCount = 0;
			timeElapsed += 1.0f;
		}

		std::string debugTitleBarText = WINDOW_CLASS_NAME;
		debugTitleBarText += "      DEBUG       Frame Stats:     Rate (FPS [Actual]): " + std::to_string(fps) + "     Rate (FPS [EMA]): " + std::to_string(m_frameRateEMA) + "     Rate (FPS [Instant]): " + std::to_string(1 / (m_frameTime + 0.000000000001)) + "     Time [EMA]: " + std::to_string(m_frameTime * 1000) + " ms" + "     Time [Mean]: " + std::to_string(1000.0f / fps) + " ms";

		SetWindowText(ref->sys->m_hWnd, debugTitleBarText.c_str());
#endif
	}

	m_clockRunning = false;

#if defined(DEBUG) || defined (_DEBUG)
	DumpD3DDebugMessagesToLog();
#endif
}

void dx12::Dx::D3D_Strings_f() const
{
	LOG_FUNC();

	if ((m_adapterDesc.VendorId == 0x1414) && (m_adapterDesc.DeviceId == 0x8c))
	{
		// Microsoft Basic Render Driver
		ref->client->Con_Printf(PRINT_ALL, L"WARNING: Microsoft Basic Render Driver is active.\n Performance of this application may be unsatisfactory.\n Please ensure that your video card is Direct3D10/11 capable\n and has the appropriate driver installed.");
	}

	ref->client->Con_Printf(PRINT_ALL, L"D3D Feature Level: " + StringForFeatureLevel(m_featureLevel));

	std::wstringstream hexValue;

	ref->client->Con_Printf(PRINT_ALL, L"    Adapter Description: " + ref->sys->ToWideString(m_adapterDesc.Description));
	ref->client->Con_Printf(PRINT_ALL, L"");

	hexValue.str(std::wstring());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.VendorId;
	ref->client->Con_Printf(PRINT_ALL, L"              Vendor ID: " + hexValue.str());

	hexValue.str(std::wstring());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.DeviceId;
	ref->client->Con_Printf(PRINT_ALL, L"              Device ID: " + hexValue.str());

	hexValue.str(std::wstring());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.SubSysId;
	ref->client->Con_Printf(PRINT_ALL, L"           Subsystem ID: " + hexValue.str());

	hexValue.str(std::wstring());
	hexValue.clear();
	hexValue << std::hex << std::showbase << m_adapterDesc.Revision;
	ref->client->Con_Printf(PRINT_ALL, L"               Revision: " + hexValue.str());

	ref->client->Con_Printf(PRINT_ALL, L" Dedicated Video Memory: " + std::to_wstring(m_adapterDesc.DedicatedVideoMemory));
	ref->client->Con_Printf(PRINT_ALL, L"Dedicated System Memory: " + std::to_wstring(m_adapterDesc.DedicatedSystemMemory));
	ref->client->Con_Printf(PRINT_ALL, L"   Shared System Memory: " + std::to_wstring(m_adapterDesc.SharedSystemMemory));
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

	if (!InitRootSignature())
	{
		LOG(error) << "Failed to create main root signature.";
		return false;
	}

	if (!InitSwapPipelineState())
	{
		LOG(error) << "Failed to create swap pipeline state.";
		return false;
	}

	if (!InitDescriptorHeaps())
	{
		LOG(error) << "Failed to create descriptor heap.";
		return false;
	}

	if (!InitViewport())
	{
		LOG(error) << "Failed to initialize viewport.";
		return false;
	}

	if (!InitScissorRect())
	{
		LOG(error) << "Failed to initialize scissor rectangle.";
		return false;
	}

	if (!InitConstantBuffer())
	{
		LOG(error) << "Failed to initialize constant buffer.";
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

	if (!InitBackBufferRenderTargets())
	{
		LOG(error) << "Failed to create back buffers.";
		return false;
	}

	if ((!subsystem2D) || (!subsystem2D->Initialize()))
	{
		LOG(error) << "Failed to create 2D overlay subsystem (GUI)";
		return false;
	}

	/*if ((!subsystemText) || (!subsystemText->Initialize()))
	{
		LOG(error) << "Failed to create text subsystem.";
		return false;
	}

	if ((!subsystem3D) || (!subsystem3D->Initialize()))
	{
		LOG(error) << "Failed to create 3D subsystem";
		return false;
	}*/

	m_testTriangle = std::make_unique<TestTriangle>();
	m_testTriangle->Initialize(m_windowWidth, m_windowHeight);

	WaitForGPU();

	return true;
}

void dx12::Dx::Shutdown()
{
	LOG_FUNC();

	if (m_testTriangle) {
		m_testTriangle->Shutdown();
		SAFE_RELEASE(m_testTriangle);
	}

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
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device5), nullptr)))
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
			ref->client->Cmd_AddCommand(L"dx12_strings", SHIM_D3D_Strings_f);
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

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		// Obtain global debug device
		hr = m_d3dDevice->QueryInterface(IID_PPV_ARGS(&d3dDebugDev));
		if (SUCCEEDED(hr))
		{
			LOG(info) << "DEBUG: Successfully created D3D Debug Device.";

			hr = d3dDebugDev->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue));
			if (SUCCEEDED(hr))
			{
				LOG(info) << "DEBUG: Successfully created D3D Debug Info Queue.";

				hr = d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

				if (FAILED(hr)) {
					LOG(error) << "DEBUG: Unable to set Info Queue break on severity CORRUPTION: " << GetD3D12ErrorMessage(hr);
				}

				hr = d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

				if (FAILED(hr)) {
					LOG(error) << "DEBUG: Unable to set Info Queue break on severity ERROR: " << GetD3D12ErrorMessage(hr);
				}

				/*hr = d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

				if (FAILED(hr)) {
					LOG(error) << "DEBUG: Unable to set Info Queue break on severity WARNING: " << GetD3D12ErrorMessage(hr);
				}*/

				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
				};

				D3D12_INFO_QUEUE_FILTER filter;
				ZeroMemory(&filter, sizeof(D3D12_INFO_QUEUE_FILTER));
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				hr = d3dInfoQueue->AddStorageFilterEntries(&filter);

				if (FAILED(hr)) {
					LOG(error) << "DEBUG: Unable to add Info Queue storage filter entries: " << GetD3D12ErrorMessage(hr);
				}

				return true;
			}
			else 
			{
				LOG(error) << "DEBUG: Unable to create D3D Debug Info Queue: " << GetD3D12ErrorMessage(hr);
			}
		}
		else
		{
			LOG(error) << "DEBUG: Unable to create D3D Debug Device: " << GetD3D12ErrorMessage(hr);
		}
	}

	LOG(error) << "Unable to create D3D device debug objects: " << GetD3D12ErrorMessage(hr);
#endif
	return false;
}

bool dx12::Dx::InitDevice(HWND hWnd)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	LOG(info) << "Creating DirectX 12 device...";

	hr = D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_d3dDevice));

	if (FAILED(hr))
	{
		LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_2, so trying D3D_FEATURE_LEVEL_12_1: " << GetD3D12ErrorMessage(hr);

		hr = D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_d3dDevice));

		if (FAILED(hr))
		{
			LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_1, so trying D3D_FEATURE_LEVEL_12_0: " << GetD3D12ErrorMessage(hr);
			hr = D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3dDevice));

			if (FAILED(hr))
			{
				LOG(warning) << "Failed to create device at D3D_FEATURE_LEVEL_12_0, so attempt to create WARP device: " << GetD3D12ErrorMessage(hr);

				ComPtr<IDXGIFactory6> dxgiFactory;
				CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

				ComPtr<IDXGIAdapter> pWarpAdapter;
				dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

				hr = D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_d3dDevice));
				if (FAILED(hr))
				{
					LOG(error) << "Failed to create WARP device: " << GetD3D12ErrorMessage(hr);
				}
				else
				{
					m_featureLevel = D3D_FEATURE_LEVEL_12_2;
				}
			}
			else
			{
				m_featureLevel = D3D_FEATURE_LEVEL_12_0;
			}
		}
		else
		{
			m_featureLevel = D3D_FEATURE_LEVEL_12_1;
		}
	}
	else 
	{
		m_featureLevel = D3D_FEATURE_LEVEL_12_2;
	}

	ref->cvars->featureLevel->Set(StringForFeatureLevel(m_featureLevel));

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created DirectX 12 device at " << StringForFeatureLevel(m_featureLevel);
	}
	else
	{
		LOG(error) << "Unable to create D3D12 device: " << GetD3D12ErrorMessage(hr);
		return false;
	}

#if defined(DEBUG) || defined (_DEBUG)
	InitDeviceDebug();
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

		hr = m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created fences.";

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (m_fenceEvent == nullptr)
		{
			LOG(error) << "Failed to create fence event.";
			DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		LOG(info) << "Successfully created fence event.";

		return true;
	}
	
	LOG(error) << "Failed to create fences.";

	return false;
}

bool dx12::Dx::InitRootSignature()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	// Describe the root parameter
	CD3DX12_DESCRIPTOR_RANGE1 srvRange = {};
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];

	rootParameters[0].InitAsConstantBufferView(0);

	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_DESCRIPTOR_RANGE1 samplerRange = {};
	samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	rootParameters[2].InitAsDescriptorTable(		1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

	// Create a static sampler
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Describe and create the root signature
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC  rootSignatureDesc = {};
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version supported. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Serialize and create the root signature
	LOG(info) << "Serializing main root signature...";

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		LOG(error) << "Failed serializing main root signature: " << GetD3D12ErrorMessage(hr) << " : " << ((char*)errorBlob->GetBufferPointer());
		return false;
	}

	LOG(info) << "Creating main root signature...";

	hr = m_d3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create main root signature: " << GetD3D12ErrorMessage(hr);
		return false;
	}

	return true;
}

bool dx12::Dx::InitSwapPipelineState()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try 
	{
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			// Position
			{
				"POSITION",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32A32_FLOAT, // Format
				0,           // InputSlot
				0,           // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Color
			{
				"COLOR",     // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32A32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Texture Coordinate 0
			{
				"TEXCOORD",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			}
		};

		LOG(info) << "Compiling Swap Chain Vertex Shader...";

		if (!m_shaderVertex.Compile(ref->cvars->shaderVertexSwap->String(), SHADER_TARGET_VERTEX))
		{
			LOG(error) << "Unable to compile Swap Chain Vertex Shader!";
			return false;
		}

		LOG(info) << "Compiling Swap Chain Pixel Shader...";

		if (!m_shaderPixel.Compile(ref->cvars->shaderPixelSwap->String(), SHADER_TARGET_PIXEL))
		{
			LOG(error) << "Unable to compile Swap Chain Pixel Shader!";
			return false;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_shaderVertex.Blob().Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_shaderPixel.Blob().Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = m_backBufferCount;
		for (int i = 0; i < m_backBufferCount; i++)
		{
			psoDesc.RTVFormats[i] = m_backBufferFormat;
		}
		psoDesc.SampleDesc.Count = 1;

		LOG(info) << "Creating Swap Chain Graphics Pipeline State Object...";

		hr = ref->sys->dx->m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create Swap Chain Graphics Pipeline State Object: " << GetD3D12ErrorMessage(hr);
			return false;
		}
	}
	catch (const std::exception& e)
	{
		LOG(error) << "Failed to create Swap Chain Graphics Pipeline State Object: " << e.what();
		return false;
	}
	return true;
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

		hr = m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Creating Swap Command List";

			m_commandListSwap = std::make_shared<CommandList>(L"Swap", m_rootSignature, m_pipelineState, m_viewport, m_scissorRect, m_constantBufferHandle);
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
		swapChainDesc.BufferCount = m_backBufferCount;
		swapChainDesc.Width = m_windowWidth;
		swapChainDesc.Height = m_windowHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = m_multisampleCount;

		ComPtr<IDXGISwapChain1> pSwapChain = nullptr;
		hr = m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);

		if (SUCCEEDED(hr))
		{
			hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));

			m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
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
		// Create descriptor heaps
		LOG(info) << "Creating descriptor heap for RTVs...";

		m_descriptorHeapRTV = std::make_shared<DescriptorHeap>(DescriptorHeap(m_d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024, false));

		if (m_descriptorHeapRTV != nullptr)
		{
			LOG(info) << "Successfully created RTV descriptor heap.";
		}
		else {
			LOG(error) << "Unable to create RTV descriptor heap.";

			return false;
		}

		LOG(info) << "Creating descriptor heap for DSVs...";

		m_descriptorHeapDSV = std::make_shared<DescriptorHeap>(DescriptorHeap(m_d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1024, false));

		if (m_descriptorHeapDSV != nullptr)
		{
			LOG(info) << "Successfully created DSV descriptor heap.";
		}
		else {
			LOG(error) << "Unable to create DSV descriptor heap.";

			return false;
		}

		LOG(info) << "Creating descriptor heap for CBV_SRV_UAVs...";

		m_descriptorHeapCBVSRVUAV = std::make_shared<DescriptorHeap>(DescriptorHeap(m_d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192, true));

		if (m_descriptorHeapCBVSRVUAV != nullptr)
		{
			LOG(info) << "Successfully created CBV_SRV_UAV descriptor heap.";
		}
		else {
			LOG(error) << "Unable to create CBV_SRV_UAV descriptor heap.";

			return false;
		}
	}

	return true;
}

bool dx12::Dx::InitBackBufferRenderTargets()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (m_d3dDevice)
	{
		// Create descriptor heaps
		LOG(info) << "Creating back buffer render targets...";

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Format = m_backBufferFormat;
		rtvDesc.Texture2D.MipSlice = 0;

		// Create a RTV for each back buffer.
		for (UINT i = 0; i < m_backBufferCount; i++)
		{
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTargets[i]));
			if (FAILED(hr))
			{
				return false;
			}

			m_backBufferRTVHandles[i] = m_descriptorHeapRTV->Allocate();
			m_d3dDevice->CreateRenderTargetView(m_backBufferRenderTargets[i].Get(), &rtvDesc, m_descriptorHeapRTV->GetCPUDescriptorHandle(m_backBufferRTVHandles[i]));
		}
	}

	if (SUCCEEDED(hr))
	{
		LOG(info) << "Successfully created back buffers.";

		return true;
	}

	LOG(error) << "Unable to create back buffers.";

	return false;
}

bool dx12::Dx::InitViewport()
{
	LOG(info) << "Setting viewport.";

	// Setup the viewport
	ZeroMemory(&m_viewport, sizeof(D3D12_VIEWPORT));
	m_viewport.Width = (FLOAT)m_windowWidth;
	m_viewport.Height = (FLOAT)m_windowHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return true;
}

bool dx12::Dx::InitScissorRect()
{
	LOG(info) << "Setting scissor rectangle.";

	// Setup the viewport
	ZeroMemory(&m_scissorRect, sizeof(D3D12_RECT));
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = static_cast<LONG>(m_windowWidth);
	m_scissorRect.bottom = static_cast<LONG>(m_windowHeight);

	return true;
}

bool dx12::Dx::InitConstantBuffer()
{
	// Create an orthographic projection matrix for 2D rendering.
	auto orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), 0.0f, 0.0f, 1.0f);

	// Create the constants buffer
	auto constantBuffer = ref->res->CreateResource<ConstantBuffer2D>(L"ConstantBufferSwap");

	if (constantBuffer)
	{
		m_constantBufferHandle = constantBuffer->GetHandle();

		ShaderConstants2D constants = {};

		// Set values
		XMStoreFloat4x4(&constants.world, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&constants.view, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&constants.projection, XMMatrixTranspose(orthoMatrix));
		constants.brightness = 0.0f;
		constants.contrast = 1.0f;

		// Create the constants buffer.
		constantBuffer->CreateConstantBuffer(&constants, sizeof(constants));

		return true;
	}

	return false;
}

void dx12::Dx::WaitForGPU()
{
	LOG_FUNC();

	if (m_commandQueue && m_fence) {
		HRESULT hr = E_UNEXPECTED;

		// Schedule a Signal command in the queue.
		hr = m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_backBufferIndex]);

		if (FAILED(hr)) {
			LOG(error) << "Unable to set signal fence: " << GetD3D12ErrorMessage(hr);
		}

		// Check if we are already done with the current procedure.
		if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex])
		{
			hr = m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent);

			if (FAILED(hr)) {
				LOG(error) << "Unable to set event fence: " << GetD3D12ErrorMessage(hr);
			}

			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Increment the fence value.
		m_fenceValues[m_backBufferIndex]++;
	}
}

void dx12::Dx::D3D_Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down D3D.";

	WaitForGPU();

	if (m_swapChain)
	{
		LOG(info) << "Switching to windowed mode to allow proper cleanup.";
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}

	if (m_commandListSwap)
	{
		m_commandListSwap->Close();
	}

	WaitForGPU();

	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		SAFE_RELEASE(m_backBufferRenderTargets[i]);
	}

	SAFE_RELEASE(m_descriptorHeapRTV);
	SAFE_RELEASE(m_descriptorHeapDSV);
	SAFE_RELEASE(m_descriptorHeapCBVSRVUAV);

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

	WaitForGPU();

	SAFE_RELEASE(m_commandListSwap);

	SAFE_RELEASE(m_pipelineState);

	SAFE_RELEASE(m_rootSignature);

	m_shaderVertex.Shutdown();

	m_shaderPixel.Shutdown();

	if (ref && ref->res)
	{
		ref->res->Shutdown();
	}

	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		SAFE_RELEASE(m_backBufferRenderTargets[i]);
	}

	WaitForGPU();

	SAFE_RELEASE(m_commandQueue);

	SAFE_RELEASE(m_fence);
	CloseHandle(m_fenceEvent);

	SAFE_RELEASE(m_swapChain);

	SAFE_RELEASE(m_d3dDevice);

	SAFE_RELEASE(m_dxgiAdapter);

	SAFE_RELEASE(m_dxgiFactory);

	m_d3dInitialized = false;
}