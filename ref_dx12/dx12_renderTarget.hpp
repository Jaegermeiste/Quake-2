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

#ifndef __DX12_RENDERTARGET_HPP__
#define __DX12_RENDERTARGET_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class RenderTarget : public Resource
	{
		friend class ResourceManager;
	private:
		D3D12_RESOURCE_STATES       m_state = D3D12_RESOURCE_STATE_COMMON;

		dxhandle_t                  m_RTVhandle = 0;
		dxhandle_t                  m_SRVhandle = 0;

		Quad2D						m_quad;
		float                       m_quadScale = 1.0f;

		DirectX::XMVECTORF32        m_clearColor = DirectX::Colors::Transparent;

		DXGI_FORMAT                 FindFormat();

	public:
		RenderTarget(std::wstring name) : Resource(name) {};
		virtual ~RenderTarget() {
			m_RTVhandle = 0;
			m_SRVhandle = 0;

			m_quad.Shutdown();
			
			if (m_resource != nullptr) {
				SAFE_RELEASE(m_resource);
			}
			m_type = RESOURCE_NONE;
		}

		bool                        CreateRenderTarget(size_t width, size_t height, DirectX::XMVECTORF32 clearColor);

		size_t                      Width() { return m_resourceDesc.Width; }
		size_t                      Height() { return m_resourceDesc.Height; }

		DXGI_FORMAT                 Format() { return m_resourceDesc.Format; };

		void                        Clear(std::shared_ptr<CommandList> commandList);

		void                        SetQuadScale(float scale) { m_quadScale = scale; };
		void                        RenderQuad(std::shared_ptr<CommandList> commandList);

		void                        Transition(std::shared_ptr<CommandList> commandList, D3D12_RESOURCE_STATES destinationState);
		D3D12_RESOURCE_STATES       State() const { return m_state; };

		D3D12_GPU_VIRTUAL_ADDRESS   GetGPUVirtualAddress() { if (m_resource) { return m_resource->GetGPUVirtualAddress(); } return 0; }

		dxhandle_t                  GetRTVHandle() const { return m_RTVhandle; };
		D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle() const;

		dxhandle_t                  GetSRVHandle() const { return m_SRVhandle; };
		D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle() const;
	};

	template std::shared_ptr<dx12::RenderTarget> dx12::ResourceManager::CreateResource<dx12::RenderTarget>(std::wstring name);
	template std::shared_ptr<dx12::RenderTarget> dx12::ResourceManager::GetOrCreateResource<dx12::RenderTarget>(std::wstring name);
	template std::shared_ptr<dx12::RenderTarget> dx12::ResourceManager::GetResource<dx12::RenderTarget>(std::wstring name);
	template std::shared_ptr<dx12::RenderTarget> dx12::ResourceManager::GetResource<dx12::RenderTarget>(dxhandle_t handle);
}

#endif//__DX12_RENDERTARGET_HPP__