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
	__declspec(align(16)) class System {
	private:
		HINSTANCE			hInstance;
		WNDPROC				wndProc;
		WNDCLASS			wndClass;
		HWND				hWnd;

		UINT						windowWidth = 0;
		UINT						windowHeight = 0;

		D3D_DRIVER_TYPE         driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11Device*           d3dDevice = nullptr;
		ID3D11Device1*          d3dDevice1 = nullptr;
		ID3D11DeviceContext*    ImmediateContext = nullptr;
		ID3D11DeviceContext1*   ImmediateContext1 = nullptr;

		IDXGISwapChain*         SwapChain = nullptr;
		IDXGISwapChain1*        SwapChain1 = nullptr;
		ID3D11RenderTargetView* RenderTargetView = nullptr;

		DirectX::XMMATRIX			world3DMatrix;
		DirectX::XMMATRIX			view3DMatrix;
		DirectX::XMMATRIX			projection3DMatrix;

		// 2D Rendering
		ID3D11DeviceContext*		deferredContext2D = nullptr;
		ID3D11Texture2D*			renderTargetTexture2D = nullptr;
		ID3D11RenderTargetView*		renderTargetView2D = nullptr;
		ID3D11ShaderResourceView*	shaderResourceView2D = nullptr;

		DirectX::XMMATRIX			m_2DorthographicMatrix;

		DirectX::XMMATRIX           m_3DworldMatrix;
		DirectX::XMMATRIX           m_3DviewMatrix;
		DirectX::XMMATRIX           m_3DprojectionMatrix;

		bool					d3dInitialized;

		bool				inRegistration;
		bool				uploadBatchOpen;

		D3D_FEATURE_LEVEL	featureLevelArray[NUM_D3D_FEATURE_LEVELS];

		void				FillFeatureLevelArray	(void);
		
		bool				VID_CreateWindow();
		void				VID_DestroyWindow();

		bool				D3D_InitDevice();
		bool				D3D_Init2DOverlay();
		void				D3D_Shutdown();

	public:
							System					();
							~System					();

		bool				Initialize(HINSTANCE hInstance, WNDPROC wndProc);
		void				Shutdown();

		void				AppActivate(bool active);

		void				BeginRegistration();
		void				EndRegistration();

		void				BeginUpload();
		void				EndUpload();

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