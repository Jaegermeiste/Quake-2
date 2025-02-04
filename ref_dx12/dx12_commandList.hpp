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
2025 Bleeding Eye Studios
*/

#ifndef __DX12_COMMANDLIST_HPP__
#define __DX12_COMMANDLIST_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class CommandList
	{
	private:
		std::wstring                        m_name = L"";
		ComPtr<ID3D12RootSignature>         m_rootSignature = nullptr;
		ComPtr<ID3D12CommandAllocator>      m_commandAllocator = nullptr;
		ComPtr<ID3D12GraphicsCommandList>	m_commandList = nullptr;
		bool						        m_commandListOpen = false;
		ComPtr<ID3D12PipelineState>         m_pipelineState = nullptr;
		D3D12_VIEWPORT                      m_viewport = {};
		D3D12_RECT                          m_scissorRect = {};
		dxhandle_t				            m_cbvHandle = 0;
		dxhandle_t				            m_rtvHandle = 0;
		dxhandle_t				            m_dsvHandle = 0;

	public:
		CommandList(std::wstring name, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12PipelineState> pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, dxhandle_t constantBufferViewHandle = 0, dxhandle_t renderTargetViewHandle = 0, dxhandle_t depthStencilViewHandle = 0);

		void Prepare();
		void SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState = nullptr);
		void Execute();
		void Close();

		ComPtr<ID3D12GraphicsCommandList>   List() { return m_commandList; };
		bool                                IsOpen() { return m_commandListOpen; };

		void Shutdown();

	};
}

#endif//__DX12_COMMANDLIST_HPP__