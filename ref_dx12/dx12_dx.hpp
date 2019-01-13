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
		UINT							m_windowWidth = 0;
		UINT							m_windowHeight = 0;

		D3D_DRIVER_TYPE					m_driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL				m_featureLevel = D3D_FEATURE_LEVEL_12_1;

		DXGI_ADAPTER_DESC				m_adapterDesc;

		D3D_FEATURE_LEVEL				m_featureLevelArray[NUM_D3D_FEATURE_LEVELS];

		LARGE_INTEGER					m_clockFrameStart;
		LARGE_INTEGER					m_clockFrameEndCurrent;
		LARGE_INTEGER					m_clockFrameEndPrevious;

		IDXGIFactory6*					m_dxgiFactory = nullptr;
		IDXGIAdapter4*					m_dxgiAdapter = nullptr;

		ID3D12Device*					m_d3dDevice = nullptr;

		ID3D12Fence*					m_fence = nullptr;
		UINT64							m_currentFence = 0;

		ID3D12CommandQueue*				m_commandQueue = nullptr;
		ID3D12CommandAllocator*			m_directCmdListAlloc = nullptr;
		ID3D12GraphicsCommandList*		m_commandListGfx = nullptr;

		IDXGISwapChain3*				m_swapChain = nullptr;

		ID3D12DescriptorHeap*			m_descriptorHeap = nullptr;
		UINT							m_descriptorsAllocated = 0;
		UINT							m_descriptorSizeRTV = 0;
		UINT							m_descriptorSizeDSV = 0;
		UINT							m_descriptorSizeCBVSRVUAV = 0;

		bool							m_d3dInitialized;

		bool							m_inRegistration;
		bool							m_uploadBatchOpen;

		bool							m_clockRunning;

		double							m_frameTime;
		double							m_frameTimeEMA;
		double							m_frameRateEMA;

		void							FillFeatureLevelArray(void);

		bool							InitFactory(HWND hWnd);
		bool							InitAdapter();
		bool							InitDeviceDebug();
		bool							InitDevice(HWND hWnd);
		bool							InitFences();
		bool							InitCommandObjects();
		bool							InitSwapChain(HWND hWnd);
		bool							InitDescriptorHeaps();

		void							D3D_Shutdown();

		friend void dx12::Draw::Fill(int x, int y, int w, int h, int c);

	public:
		std::unique_ptr<Subsystem2D>	subsystem2D;
		std::unique_ptr<Subsystem3D>	subsystem3D;
		std::unique_ptr<SubsystemText>	subsystemText;

	private:
		byte							m_padding[4];

	public:
										Dx();

		bool							Initialize(HWND hWnd);
		void							Shutdown();

		void							BeginFrame();
		void							RenderFrame(refdef_t *fd);
		void							EndFrame();

		// Commands
		void							D3D_Strings_f();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_DX_HPP__