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

#define NUM_D3D_FEATURE_LEVELS	9

#define MIN_BACK_BUFFERS	1
#define MAX_BACK_BUFFERS	3

#include "dx12_local.hpp"

namespace dx12
{
	//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
	__declspec(align(16)) class Dx
	{
		friend class Quad2D;
		friend class SubsystemText;
		friend class Subsystem2D;
		friend class Subsystem3D;
		friend class Draw;
		friend class ImageManager;
	private:
		UINT										m_windowWidth					= 0;
		UINT										m_windowHeight					= 0;

		D3D_DRIVER_TYPE								m_driverType					= D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL							m_featureLevel					= D3D_FEATURE_LEVEL_12_1;

		DXGI_ADAPTER_DESC							m_adapterDesc{};

		D3D_FEATURE_LEVEL							m_featureLevelArray[NUM_D3D_FEATURE_LEVELS]{};

		LARGE_INTEGER								m_clockFrameStart{};
		LARGE_INTEGER								m_clockFrameEndCurrent{};
		LARGE_INTEGER								m_clockFrameEndPrevious{};

		std::shared_ptr<IDXGIFactory6>				m_dxgiFactory					= nullptr;
		std::shared_ptr<IDXGIAdapter4>				m_dxgiAdapter					= nullptr;

		std::shared_ptr<ID3D12Device>				m_d3dDevice						= nullptr;

		std::shared_ptr<ID3D12Fence>				m_fence							= nullptr;
		HANDLE										m_fenceEvent;
		UINT64										m_fenceValues[MAX_BACK_BUFFERS]{};

		std::shared_ptr<ID3D12CommandQueue>			m_commandQueue					= nullptr;
		std::shared_ptr<ID3D12CommandAllocator>		m_directCmdListAllocs[MAX_BACK_BUFFERS]{};
		std::shared_ptr<ID3D12GraphicsCommandList>	m_commandListGfx				= nullptr;

		UINT										m_multisampleCount				= 0;
		std::shared_ptr<IDXGISwapChain3>			m_swapChain						= nullptr;

		std::shared_ptr<ID3D12DescriptorHeap>		m_descriptorHeapRTV				= nullptr;
		std::shared_ptr<ID3D12DescriptorHeap>		m_descriptorHeapDSV				= nullptr;
		std::shared_ptr<ID3D12DescriptorHeap>		m_descriptorHeapCBVSRVUAV		= nullptr;
		UINT										m_descriptorsAllocatedRTV		= 0;
		UINT										m_descriptorsAllocatedDSV		= 0;
		UINT										m_descriptorsAllocatedCBVSRVUAV	= 0;
		UINT										m_descriptorSizeRTV				= 0;
		UINT										m_descriptorSizeDSV				= 0;
		UINT										m_descriptorSizeCBVSRVUAV		= 0;

		UINT										m_backBufferCount				= 0;
		std::shared_ptr<ID3D12Resource>				m_backBufferRenderTargets[MAX_BACK_BUFFERS]{};
		UINT										m_backBufferIndex				= 0;

		D3D12_VIEWPORT								m_viewport{};
		D3D12_RECT									m_scissorRect{};

		bool										m_d3dInitialized				= false;

		bool										m_inRegistration				= false;
		bool										m_uploadBatchOpen				= false;

		bool										m_clockRunning					= false;

		double										m_frameTime						= 0.0;
		double										m_frameTimeEMA					= 0.0;
		double										m_frameRateEMA					= 0.0;

		void										FillFeatureLevelArray(void);

		bool										InitFactory(HWND hWnd);
		bool										InitAdapter();
		bool										InitDeviceDebug();
		bool										InitDevice(HWND hWnd);
		bool										InitFences();
		bool										InitCommandObjects();
		bool										InitSwapChain(HWND hWnd);
		bool										InitDescriptorHeaps();
		bool										InitBackBufferRenderTargets();
		bool										InitViewport();
		bool										InitScissorRect();

		void										WaitForGPU();

		void										D3D_Shutdown();

		friend void dx12::Draw::Fill(int x, int y, int w, int h, int c);
		friend std::shared_ptr<dx12::Texture2D> dx12::ImageManager::CreateTexture2DFromRaw(std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR *palette);

	public:
		std::unique_ptr<Subsystem2D>				subsystem2D;
		std::unique_ptr<Subsystem3D>				subsystem3D;
		std::unique_ptr<SubsystemText>				subsystemText;

	protected:
		std::shared_ptr<ID3D12DescriptorHeap>		GetHeapRTV() { return m_descriptorHeapRTV; };
		std::shared_ptr<ID3D12DescriptorHeap>		GetHeapDSV() { return m_descriptorHeapDSV; };
		std::shared_ptr<ID3D12DescriptorHeap>		GetHeapCBVSRVUAV() { return m_descriptorHeapCBVSRVUAV; };

	public:
													Dx();

		bool										Initialize(HWND hWnd);
		void										Shutdown();

		void										BeginFrame();
		void										RenderFrame(refdef_t *fd);
		void										EndFrame();

		// Commands
		void										D3D_Strings_f();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_DX_HPP__