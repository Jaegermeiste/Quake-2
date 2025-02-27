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

#ifndef __DX12_BLASBUFFER_HPP__
#define __DX12_BLASBUFFER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class BLASBuffer : public Resource
	{
		friend class ResourceManager;

	private:
		D3D12_RAYTRACING_GEOMETRY_DESC                        m_geometryDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS  m_blasInputs = {};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_blasPrebuildInfo = {};

	protected:
		unsigned int            m_count = 0;

		ComPtr<ID3D12Resource>	m_scratch = nullptr;
		unsigned int            m_scratchCount = 0;

		bool CreateBuffers(std::shared_ptr<CommandList> commandList);

		bool BuildBuffers(std::shared_ptr<CommandList> commandList);

	public:
		BLASBuffer(std::wstring name) : Resource(name)
		{
			m_type = RESOURCE_BLASBUFFER;
			m_count = 0;
		};

		bool Create(std::shared_ptr<CommandList> commandList, dxhandle_t vertexBufferResource, dxhandle_t indexBufferResource);

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() { if (m_resource) { return m_resource->GetGPUVirtualAddress(); } return 0; }
		D3D12_GPU_VIRTUAL_ADDRESS GetScratchGPUVirtualAddress() { if (m_scratch) { return m_scratch->GetGPUVirtualAddress(); } return 0; }
	};

	template std::shared_ptr<dx12::BLASBuffer> dx12::ResourceManager::CreateResource<dx12::BLASBuffer>(std::wstring name);
	template std::shared_ptr<dx12::BLASBuffer> dx12::ResourceManager::GetOrCreateResource<dx12::BLASBuffer>(std::wstring name);
	template std::shared_ptr<dx12::BLASBuffer> dx12::ResourceManager::GetResource<dx12::BLASBuffer>(std::wstring name);
	template std::shared_ptr<dx12::BLASBuffer> dx12::ResourceManager::GetResource<dx12::BLASBuffer>(dxhandle_t handle);
}

#endif//__DX12_BLASBUFFER_HPP__