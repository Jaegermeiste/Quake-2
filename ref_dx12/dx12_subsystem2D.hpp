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
		ComPtr<ID3D12RootSignature>           m_rootSignature = nullptr;
		std::shared_ptr<CommandList>    	  m_commandList = nullptr;
		ComPtr<ID3D12PipelineState>           m_pipelineState = nullptr;
		std::shared_ptr<RenderTarget> m_renderTarget = nullptr;
		D3D12_VIEWPORT                        m_viewport = {};
		D3D12_RECT                            m_scissorRect = {};

		DirectX::XMMATRIX			          m_2DorthographicMatrix;

		Shader						          m_2DshaderVertex;
		Shader						          m_2DshaderPixel;

		Quad2D						          m_fadeScreenQuad;

		dxhandle_t				              m_constantBufferHandle = 0;

		bool                                  CreateRootSignatures();

		bool                                  CompileShaders();

		bool                                  CreateGraphicsPipelineStateObject();

		bool                                  CreateConstantsBuffer();

		bool                                  CreateRenderTarget();

	public:
			  						          Subsystem2D();

		bool						          Initialize();

		void						          Clear();
		 
		void						          Update();

		void						          Render(std::shared_ptr<CommandList> commandListSwap);

		void						          FadeScreen();

		void						          Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_2D_HPP__