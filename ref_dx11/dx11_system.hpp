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

#ifndef __DX11_SYSTEM_HPP__
#define __DX11_SYSTEM_HPP__
#pragma once

#define NUM_D3D_FEATURE_LEVELS	9

#define	WINDOW_CLASS_NAME	"Quake 2"

#include "dx11_local.hpp"

namespace dx11
{
	//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
	__declspec(align(16)) class System 
	{
		friend class Subsystem2D;
	private:
		HINSTANCE						m_hInstance;
		WNDPROC							m_wndProc;
		WNDCLASSEX						m_wndClassEx;
		HWND							m_hWnd;

		UINT							m_windowWidth = 0;
		UINT							m_windowHeight = 0;

		D3D_DRIVER_TYPE					m_driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL				m_featureLevel = D3D_FEATURE_LEVEL_12_1;

		DXGI_ADAPTER_DESC				m_adapterDesc;

		D3D_FEATURE_LEVEL				m_featureLevelArray[NUM_D3D_FEATURE_LEVELS];

		byte							m_padding[4];

		// Timing
		LARGE_INTEGER					m_clockFrequency;
		LARGE_INTEGER					m_clockFrameStart;
		LARGE_INTEGER					m_clockFrameEndCurrent;
		LARGE_INTEGER					m_clockFrameEndPrevious;

		ID3D11Device*					m_d3dDevice = nullptr;
		ID3D11Device1*					m_d3dDevice1 = nullptr;
		ID3D11DeviceContext*			m_immediateContext = nullptr;
		ID3D11DeviceContext1*			m_immediateContext1 = nullptr;

		IDXGISwapChain*					m_swapChain = nullptr;
		IDXGISwapChain1*				m_swapChain1 = nullptr;

		ID3D11DepthStencilView*			m_depthStencilView;
		
		// 2D Rendering
		std::unique_ptr<Subsystem2D>	m_overlaySystem;


		// 3D Rendering
		ID3D11RenderTargetView*			m_backBufferRTV = nullptr;
		ID3D11DepthStencilState*		m_depthStencilState = nullptr;
		DirectX::XMMATRIX				m_3DworldMatrix;
		DirectX::XMMATRIX				m_3DviewMatrix;
		DirectX::XMMATRIX				m_3DprojectionMatrix;

		bool							m_d3dInitialized;

		bool							m_inRegistration;
		bool							m_uploadBatchOpen;

		bool							m_clockRunning;
		bool							m_clockFrequencyObtained;

		byte							m_padding2[3];

		double							m_frameTime;
		double							m_frameTimeEMA;
		double							m_frameRateEMA;


		void							FillFeatureLevelArray	(void);
		
		bool							VID_CreateWindow();
		void							VID_DestroyWindow();

		bool							D3D_InitDevice();

		void							D3D_Shutdown();

	public:
									System					();

		bool						Initialize(HINSTANCE hInstance, WNDPROC wndProc);
		void						Shutdown();

		void						AppActivate(bool active);

		void						BeginRegistration();
		void						EndRegistration();

		void						BeginUpload();
		void						EndUpload();

		void						BeginFrame();
		void						RenderFrame(refdef_t *fd);
		void						EndFrame();

		void						D3D_Strings_f();

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

#endif // !__DX11_SYSTEM_HPP__