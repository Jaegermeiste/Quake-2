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

#ifndef __DX11_2D_HPP__
#define __DX11_2D_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	__declspec(align(16)) struct ShaderConstants2D
	{
		float brightness;
		float contrast;
		float unused1;
		float unused2;
	};

	__declspec(align(16)) class Subsystem2D {
		friend class System;
		friend class SubsystemText;
		friend class Draw;
		friend class Dx;
		friend class Quad2D;
	private:
		UINT						m_renderTargetWidth = 0;
		UINT						m_renderTargetHeight = 0;

		ID3D11DeviceContext*		m_2DdeferredContext = nullptr;
		ID3D11CommandList*			m_2DcommandList = nullptr;
		ID3D11Texture2D*			m_2DrenderTargetTexture = nullptr;
		ID3D11RenderTargetView*		m_2DoverlayRTV = nullptr;
		ID3D11ShaderResourceView*	m_2DshaderResourceView = nullptr;
		ID3D11DepthStencilState*	m_depthDisabledStencilState = nullptr;
		IDXGISurface*				m_dxgiSurface = nullptr;
		ID2D1RenderTarget*			m_d2dRenderTarget = nullptr;
		ID2D1SolidColorBrush*		fadeColor = nullptr;

		bool						m_d2dDrawingActive = false;

		byte						m_padding[3];

		DirectX::XMMATRIX			m_2DorthographicMatrix;

		Shader						m_2DshaderVertexColor;
		Shader						m_2DshaderTexture;

		ID3D11UnorderedAccessView*	m_2DunorderedAccessView = nullptr;
		
		ID3D11BlendState*			m_alphaBlendState = nullptr;

		Quad2D						m_renderTargetQuad;
		Quad2D						m_generalPurposeQuad;

		void						EndD2DDrawing();
		
	public:
		ID2D1SolidColorBrush*		colorBlack = nullptr;
		ID2D1SolidColorBrush*		colorGray = nullptr;
		ID2D1SolidColorBrush*		colorYellowGreen = nullptr;
		ID2D1SolidColorBrush*		colorWhite = nullptr;
		ID2D1SolidColorBrush*		colorRed = nullptr;
		ID2D1SolidColorBrush*		colorBlue = nullptr;

	private:
		ID3D11Buffer*				m_constantBuffer = nullptr;
		byte						m_padding2[4];

	public:
									Subsystem2D();

		bool						Initialize();

		void						ActivateD2DDrawing();

		void						Clear();

		void						Update();

		void						Render();

		void						FadeScreen();

		void						Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX11_2D_HPP__