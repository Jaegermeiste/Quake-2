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

#ifndef __DX12_SHADERBINDINGTABLE_HPP__
#define __DX12_SHADERBINDINGTABLE_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class ShaderBindingRecord
	{
	private:
		void*    m_shaderID = nullptr;
		size_t   m_shaderIDSize = 0;
		byte*    m_rootArguments = nullptr;
		size_t   m_rootArgumentsSize = 0;

	public:
		ShaderBindingRecord(void* shaderIdentifier, size_t shaderIdentifierSize)
		{
			m_shaderID = shaderIdentifier;
			m_shaderIDSize = AlignUp(shaderIdentifierSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		}

		ShaderBindingRecord(void* shaderIdentifier, size_t shaderIdentifierSize, byte* rootArguments, size_t rootArgumentsSize)
		{
			m_shaderID = shaderIdentifier;
			m_shaderIDSize = AlignUp(shaderIdentifierSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			m_rootArguments = rootArguments;
			m_rootArgumentsSize = rootArgumentsSize;
		}

		void CopyTo(byte* destination) const
		{
			memcpy(destination, m_shaderID, m_shaderIDSize);

			if (m_rootArguments)
			{
				memcpy(destination + m_shaderIDSize, m_rootArguments, m_rootArgumentsSize);
			}
		}
	};

	class ShaderBindingTable : public ResourceBuffer
	{
		friend class ResourceManager;
	private:
		size_t                             m_shaderRecordSize = 0;

		std::unique_ptr<byte*>             m_shaderRecordsBuffer;
		size_t                             m_bufferSize = 0;

		std::vector<ShaderBindingRecord>   m_shaderRecords;

	public:
		ShaderBindingTable(std::wstring name) : ResourceBuffer(name) {};

		bool                        CreateTable(std::wstring shaderName, ComPtr<ID3D12StateObject> pipelineState, byte* rootArguments, size_t rootArgumentsSize);

		D3D12_GPU_VIRTUAL_ADDRESS   GetGPUVirtualAddress() { if (m_resource) { return m_resource->GetGPUVirtualAddress(); } return 0; }
	
		size_t                      GetShaderRecordSize() const { return m_shaderRecordSize; };
		size_t                      GetShaderStride() const { return m_shaderRecordSize; };
		size_t                      GetShaderIdentifierSize() { return AlignUp(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT); };
		size_t                      GetNumShaderRecords() { return m_count; };
	};
}

template bool dx12::ResourceBuffer::CreateBuffer<byte>(byte* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UploadBuffer<byte>(byte* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UpdateBuffer<byte>(std::shared_ptr<CommandList> commandList, byte* bufferData, size_t bufferSize);

template std::shared_ptr<dx12::ShaderBindingTable> dx12::ResourceManager::CreateResource<dx12::ShaderBindingTable>(std::wstring name);
template std::shared_ptr<dx12::ShaderBindingTable> dx12::ResourceManager::GetOrCreateResource<dx12::ShaderBindingTable>(std::wstring name);
template std::shared_ptr<dx12::ShaderBindingTable> dx12::ResourceManager::GetResource<dx12::ShaderBindingTable>(std::wstring name);
template std::shared_ptr<dx12::ShaderBindingTable> dx12::ResourceManager::GetResource<dx12::ShaderBindingTable>(dxhandle_t handle);

#endif//__DX12_SHADERBINDINGTABLE_HPP__