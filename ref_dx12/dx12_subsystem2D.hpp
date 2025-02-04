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

#ifndef __DX12_2D_HPP__
#define __DX12_2D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	__declspec(align(16)) class Subsystem2D {
		friend class System;
		friend class SubsystemText;
		friend class Draw;
		friend class Dx;
		friend class Quad2D;
	private:
		UINT						        m_renderTargetWidth = 0;
		UINT						        m_renderTargetHeight = 0;

		ComPtr<ID3D12RootSignature>         m_rootSignature = nullptr;
		std::shared_ptr<CommandList>    	m_commandList = nullptr;
		ComPtr<ID3D12PipelineState>         m_pipelineState = nullptr;
		ComPtr<ID3D12Resource>		        m_2DrenderTargetTexture = nullptr;
		D3D12_RESOURCE_STATES               m_2drenderTargetState = D3D12_RESOURCE_STATE_COMMON;
		dxhandle_t                          m_2dRTVHandle = {};
		dxhandle_t                          m_2dDSVHandle = {};
		dxhandle_t                          m_2dSRVHandle = {};
		D3D12_VIEWPORT                      m_viewport = {};
		D3D12_RECT                          m_scissorRect = {};

		DirectX::XMMATRIX			        m_2DorthographicMatrix;

		Shader						        m_2DshaderVertex;
		Shader						        m_2DshaderPixel;

		Quad2D						        m_renderTargetQuad;
		Quad2D						        m_generalPurposeQuad;
		Quad2D						        m_fadeScreenQuad;

		dxhandle_t				            m_constantBufferHandle = 0;

	public:
									        Subsystem2D();

		bool						        Initialize();

		void						        Clear();
		 
		void						        Update();

		void						        Render();

		void						        FadeScreen();

		void						        Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_2D_HPP__