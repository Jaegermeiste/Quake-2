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
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;

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
	std::string featureLevelString = "D3D_FEATURE_LEVEL_12_1";

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
	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		m_directCmdListAllocs[i] = nullptr;
	}
	m_commandListGfx = nullptr;

	m_multisampleCount = ref->cvars->samplesPerPixel->UInt();
	m_swapChain = nullptr;

	m_descriptorHeap = nullptr;
	m_descriptorsAllocated = 0;
	m_descriptorSizeRTV = 0;
	m_descriptorSizeDSV = 0;
	m_descriptorSizeCBVSRVUAV = 0;

	m_backBufferCount = ref->cvars->bufferCount->Int();
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

	// Timing
	if ((ref->sys->m_clockFrequencyObtained) && (QueryPerformanceCounter(&m_clockFrameStart) == TRUE))
	{
		m_clockRunning = true;
	}
	else
	{
		m_clockRunning = false;
	}

	DX::ThrowIfFailed(m_directCmdListAllocs[m_backBufferIndex]->Reset());

	DX::ThrowIfFailed(m_commandListGfx->Reset(m_directCmdListAllocs[m_backBufferIndex], nullptr));

	//m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandListGfx->RSSetViewports(1, &m_viewport);
	m_commandListGfx->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandListGfx->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTargets[m_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	if (ref->cvars->clear->Bool())
	{
		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

		/*
		// Induce Seizures to visualize back buffer flip
		if (m_backBufferIndex % 2)
		{
			clearColor[0] = 0.4f;
			clearColor[2] = 0.0f;
		}
		*/

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex, m_descriptorSizeRTV);
		m_commandListGfx->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}

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

	// Indicate that the back buffer will now be used to present.
	m_commandListGfx->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTargets[m_backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Cloase the command list
	m_commandListGfx->Close();

	if (m_commandQueue)
	{
		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandListGfx };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	if (m_swapChain)
	{
		// Switch the back buffer and the front buffer
		m_swapChain->Present(ref->cvars->Vsync->UInt(), 0);

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
		DX::ThrowIfFailed(m_commandQueue->Signal(m_fence, currentFenceValue));

		// Update the frame index.
		m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex])
		{
			DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent));
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;
	}

	// Timing
	if ((m_clockRunning) && (QueryPerformanceCounter(&m_clockFrameEndCurrent) == TRUE))
	{
		m_frameTime = static_cast<double>(m_clockFrameEndCurrent.QuadPart - m_clockFrameStart.QuadPart) / ref->sys->m_clockFrequency.QuadPart;

		m_frameTimeEMA = EMA_ALPHA * m_frameTimeEMA + (1.0 - EMA_ALPHA) * m_frameTime;

		m_frameRateEMA = EMA_ALPHA * m_frameRateEMA + (1.0 - EMA_ALPHA) * (1 / (m_frameTime + 0.000000000001));
		m_clockFrameEndPrevious = m_clockFrameEndCurrent;

		//LOG(trace) << "Frame <rate> " << m_frameRateEMA << " fps <time> " << m_frameTime << " ms";
#ifdef _DEBUG
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

	if (!InitDescriptorHeaps())
	{
		LOG(error) << "Failed to create descriptor heap.";
		return false;
	}

	if (!InitBackBufferRenderTargets())
	{
		LOG(error) << "Failed to create back buffers.";
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

	WaitForGPU();

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
			LOG(info) << "Creating Command Allocators";

			for (unsigned int i = 0; i < m_backBufferCount; i++)
			{
				hr = m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdListAllocs[i]));
				if (FAILED(hr))
				{
					LOG(error) << "Failed to create Command Allocator " << i + 1 << ".";
				}
			}

			if (SUCCEEDED(hr))
			{
				LOG(info) << "Creating Graphics Command List";

				hr = m_d3dDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					m_directCmdListAllocs[0], // Associated command allocator
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
				LOG(error) << "Failed to create Command Allocators.";
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
		swapChainDesc.BufferCount = m_backBufferCount;
		swapChainDesc.Width = m_windowWidth;
		swapChainDesc.Height = m_windowHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = m_multisampleCount;

		IDXGISwapChain1* pSwapChain = nullptr;
		hr = m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue, hWnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);

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
		// Get descriptor sizes
		LOG(info) << "Obtaining descriptor heap element sizes...";
		m_descriptorSizeRTV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_RTV: " << m_descriptorSizeRTV;

		m_descriptorSizeDSV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_DSV: " << m_descriptorSizeDSV;

		m_descriptorSizeCBVSRVUAV = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		LOG(info) << "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: " << m_descriptorSizeCBVSRVUAV;

		// Create descriptor heaps
		LOG(info) << "Creating descriptor heap for back buffer RTVs...";

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

		descriptorHeapDesc.NumDescriptors = m_backBufferCount;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
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
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.Texture2D.MipSlice = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		// Create a RTV for each back buffer.
		for (UINT i = 0; i < m_backBufferCount; i++)
		{
			hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTargets[i]));
			if (FAILED(hr))
			{
				return false;
			}

			m_d3dDevice->CreateRenderTargetView(m_backBufferRenderTargets[i], &rtvDesc, rtvHandle);
			rtvHandle.ptr += m_descriptorSizeRTV;
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

void dx12::Dx::WaitForGPU()
{
	// Schedule a Signal command in the queue.
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence, m_fenceValues[m_backBufferIndex]));

	// Wait until the fence has been processed.
	DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_backBufferIndex]++;
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

	WaitForGPU();

	CloseHandle(m_fenceEvent);

	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		SAFE_RELEASE(m_backBufferRenderTargets[i]);
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

	for (unsigned int i = 0; i < MAX_BACK_BUFFERS; i++)
	{
		SAFE_RELEASE(m_directCmdListAllocs[i]);
	}

	SAFE_RELEASE(m_commandQueue);

	SAFE_RELEASE(m_fence);

	SAFE_RELEASE(m_d3dDevice);

	SAFE_RELEASE(m_dxgiAdapter);

	SAFE_RELEASE(m_dxgiFactory);

	m_d3dInitialized = false;
}