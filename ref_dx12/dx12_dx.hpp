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

#ifndef __DX12_DX_HPP__
#define __DX12_DX_HPP__
#pragma once

constexpr auto NUM_D3D_FEATURE_LEVELS = 10;

constexpr auto MIN_BACK_BUFFERS = 1;
constexpr auto MAX_BACK_BUFFERS = 3;

#include "dx12_local.hpp"

namespace dx12
{
	//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
	__declspec(align(16)) class Dx
	{
		friend class CommandList;
		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class TestTriangle;
		friend class Quad2D;
		friend class SubsystemText;
		friend class Subsystem2D;
		friend class Subsystem3D;
		friend class Draw;
		friend class ImageManager;
	private:
		UINT										m_windowWidth					= 0;
		UINT										m_windowHeight					= 0;

		UINT										m_modeWidth                     = 0;
		UINT										m_modeHeight                    = 0;

		D3D_DRIVER_TYPE								m_driverType					= D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL							m_featureLevel					= D3D_FEATURE_LEVEL_12_2;

		DXGI_ADAPTER_DESC							m_adapterDesc{};

		D3D_FEATURE_LEVEL							m_featureLevelArray[NUM_D3D_FEATURE_LEVELS]{};

		LARGE_INTEGER								m_clockFrameStart{};
		LARGE_INTEGER								m_clockFrameEndCurrent{};
		LARGE_INTEGER								m_clockFrameEndPrevious{};

		ComPtr<IDXGIFactory6>				        m_dxgiFactory = nullptr;
		ComPtr<IDXGIAdapter4>				        m_dxgiAdapter					= nullptr;

		ComPtr<ID3D12Device14>				        m_d3dDevice						= nullptr;

		ComPtr<ID3D12Fence>				            m_fence							= nullptr;
		HANDLE										m_fenceEvent;
		UINT64										m_fenceValues[MAX_BACK_BUFFERS]{};

		ComPtr<ID3D12CommandQueue>			        m_commandQueue					= nullptr;
		std::shared_ptr<CommandList>      	        m_commandListSwap				= nullptr;

		UINT										m_multisampleCount				= 0;
		ComPtr<IDXGISwapChain3>			            m_swapChain                     = nullptr;

		std::shared_ptr<DescriptorHeap>		        m_descriptorHeapRTV				= nullptr;
		std::shared_ptr<DescriptorHeap>		        m_descriptorHeapDSV				= nullptr;
		std::shared_ptr<DescriptorHeap>		        m_descriptorHeapCBVSRVUAV		= nullptr;

		ComPtr<ID3D12RootSignature>                 m_rootSignature                 = nullptr;
		ComPtr<ID3D12PipelineState>                 m_pipelineState                 = nullptr;

		dxhandle_t				                    m_constantBufferHandle = 0;

		DXGI_FORMAT									m_backBufferFormat              = DXGI_FORMAT_R8G8B8A8_UNORM;

		UINT										m_backBufferCount				= 0;
		ComPtr<ID3D12Resource>				        m_backBufferRenderTargets[MAX_BACK_BUFFERS]{};
		UINT										m_backBufferIndex				= 0;
		D3D12_RESOURCE_STATES                       m_backBufferRenderTargetStates[MAX_BACK_BUFFERS] = {};
		dxhandle_t                                  m_backBufferRTVHandles[MAX_BACK_BUFFERS] = {};

		D3D12_VIEWPORT								m_viewport                      = {};
		D3D12_RECT									m_scissorRect                   = {};

		CD3DX12FeatureSupport                       m_featureSupport                = {};
		bool                                        m_tightAlignment                = false;

		bool										m_d3dInitialized				= false;

		bool										m_inRegistration				= false;
		bool										m_uploadBatchOpen				= false;

		bool										m_clockRunning					= false;

		double										m_frameTime						= 0.0;
		double										m_frameTimeEMA					= 0.0;
		double										m_frameRateEMA					= 0.0;

		Shader						                m_shaderVertex;
		Shader						                m_shaderPixel;

		void										FillFeatureLevelArray(void);

		bool										InitFactory(HWND hWnd);
		bool										InitAdapter();
		bool										InitDeviceDebug();
		bool										InitDevice(HWND hWnd);
		bool                                        InitFeatures();
		bool										InitFences();
		bool										InitRootSignature();
		bool										InitSwapPipelineState();
		bool										InitCommandObjects();
		bool										InitSwapChain(HWND hWnd);
		bool										InitDescriptorHeaps();
		bool										InitBackBufferRenderTargets();
		bool										InitViewport();
		bool										InitScissorRect();
		bool										InitConstantBuffer();

		std::unique_ptr<TestTriangle>               m_testTriangle = nullptr;

		void										D3D_Shutdown();

		friend void dx12::Draw::Fill(int x, int y, int w, int h, int c);

	public:
		std::unique_ptr<Subsystem2D>				subsystem2D;
		std::unique_ptr<Subsystem3D>				subsystem3D;
		std::unique_ptr<SubsystemText>				subsystemText;

	protected:
		void										WaitForGPU();

	public:
													Dx();

		bool										Initialize(HWND hWnd);
		void										Shutdown();

		void										BeginFrame();
		void										RenderFrame(refdef_t *fd) const;
		void										EndFrame();

		ComPtr<ID3D12Device14>                      Device() { return m_d3dDevice; };
		std::shared_ptr<DescriptorHeap>		        HeapRTV() { return m_descriptorHeapRTV; };
		std::shared_ptr<DescriptorHeap>		        HeapDSV() { return m_descriptorHeapDSV; };
		std::shared_ptr<DescriptorHeap>		        HeapCBVSRVUAV() { return m_descriptorHeapCBVSRVUAV; };

		D3D_FEATURE_LEVEL                           FeatureLevel() const { return m_featureLevel; };

		bool                                        HasTightAlignment() const { return m_tightAlignment; };

		unsigned int                                WindowWidth() const { return m_windowWidth; };
		unsigned int                                WindowHeight() const { return m_windowHeight; };

		unsigned int                                ModeWidth() const { return m_modeWidth; };
		unsigned int                                ModeHeight() const { return m_modeHeight; };


		// Commands
	 	void										D3D_Strings_f() const;
		void										D3D_Memory_f();
		void										D3D_Capabilities_f();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_DX_HPP__