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

#ifndef __DX12_RAYTRACINGBUFFER_HPP__
#define __DX12_RAYTRACINGBUFFER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class RaytracingBuffer : public Resource
	{
		friend class ResourceManager;
	private:
		dxhandle_t               	m_uavHandle = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cachedD3D12UAVHandle = {};

		size_t                      m_width = 0;
		size_t                      m_height = 0;

		void                        RefreshUAV();

	protected:
		size_t                      m_count = 0;

	public:
		                            RaytracingBuffer(std::wstring name);

		bool                        CreateBuffer(size_t width, size_t height);

		void                        CreateUAV();
		void                        BindUAV(std::shared_ptr<CommandList> commandList);

		size_t                      Width() { return m_width; }
		size_t                      Height() { return m_height; }

		D3D12_GPU_VIRTUAL_ADDRESS   GetGPUVirtualAddress() { if (m_resource) { return m_resource->GetGPUVirtualAddress(); } return 0; }
	};

	template std::shared_ptr<dx12::RaytracingBuffer> dx12::ResourceManager::CreateResource<dx12::RaytracingBuffer>(std::wstring name);
	template std::shared_ptr<dx12::RaytracingBuffer> dx12::ResourceManager::GetOrCreateResource<dx12::RaytracingBuffer>(std::wstring name);
	template std::shared_ptr<dx12::RaytracingBuffer> dx12::ResourceManager::GetResource<dx12::RaytracingBuffer>(std::wstring name);
	template std::shared_ptr<dx12::RaytracingBuffer> dx12::ResourceManager::GetResource<dx12::RaytracingBuffer>(dxhandle_t handle);
}

#endif//__DX12_RAYTRACINGBUFFER_HPP__