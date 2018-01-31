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

#ifndef __DX11_DX_HPP__
#define __DX11_DX_HPP__
#pragma once

#define NUM_D3D_FEATURE_LEVELS	9

#include "dx11_local.hpp"

namespace dx11
{
	__declspec(align(16)) struct Vertex2D
	//struct alignas(16) Vertex2D
	//struct Vertex2D
	{
		DirectX::XMFLOAT4A		position;
		DirectX::XMVECTORF32	color;
		DirectX::XMFLOAT2A		texCoord;
		/*
		//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_mm_free(p);
		}
		*/
	};

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

		ID2D1Factory1*					m_d2dFactory = nullptr;
		ID2D1Device*					m_d2dDevice = nullptr;
		ID2D1DeviceContext*				m_d2dContext = nullptr;
		ID2D1CommandList*				m_d2dCommandList = nullptr;
		ID3D11Device*					m_d3dDevice = nullptr;
		ID3D11Device1*					m_d3dDevice1 = nullptr;
		ID3D11DeviceContext*			m_immediateContext = nullptr;
		ID3D11DeviceContext1*			m_immediateContext1 = nullptr;

		IDXGISwapChain*					m_swapChain = nullptr;
		IDXGISwapChain1*				m_swapChain1 = nullptr;

		ID3D11DepthStencilView*			m_depthStencilView;

		ID3D11RenderTargetView*			m_backBufferRTV = nullptr;
		ID3D11DepthStencilState*		m_depthStencilState = nullptr;

		bool							m_d3dInitialized;

		bool							m_inRegistration;
		bool							m_uploadBatchOpen;

		bool							m_clockRunning;

	
		double							m_frameTime;
		double							m_frameTimeEMA;
		double							m_frameRateEMA;

		void							FillFeatureLevelArray(void);

		bool							InitDevice(HWND hWnd);

		void							D3D_Shutdown();

		friend void dx11::Draw::Fill(int x, int y, int w, int h, int c);

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

		//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
}

#endif // !__DX11_DX_HPP__