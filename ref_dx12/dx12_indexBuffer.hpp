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

#ifndef __DX12_INDEXBUFFER_HPP__
#define __DX12_INDEXBUFFER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class IndexBuffer : public ResourceBuffer
	{
		friend class ResourceManager;

	public:
		IndexBuffer(std::wstring name) : ResourceBuffer(name)
		{
			m_type = RESOURCE_INDEXBUFFER;
			m_count = 0;
		};

		bool CreateIndexBuffer(unsigned long* bufferData, size_t bufferSize) {
			return CreateBuffer(bufferData, bufferSize);
		}

		bool UpdateIndexBuffer(std::shared_ptr<CommandList> commandList, unsigned long* bufferData, size_t bufferSize) {
			return UpdateBuffer(commandList, bufferData, bufferSize);
		}

		size_t		IndexCount() { return m_count; };
	};
}

template bool dx12::ResourceBuffer::CreateBuffer<unsigned long>(unsigned long* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UploadBuffer<unsigned long>(unsigned long* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UpdateBuffer<unsigned long>(std::shared_ptr<CommandList> commandList, unsigned long* bufferData, size_t bufferSize);

template std::shared_ptr<dx12::IndexBuffer> dx12::ResourceManager::CreateResource<dx12::IndexBuffer>(std::wstring name);
template std::shared_ptr<dx12::IndexBuffer> dx12::ResourceManager::GetOrCreateResource<dx12::IndexBuffer>(std::wstring name);
template std::shared_ptr<dx12::IndexBuffer> dx12::ResourceManager::GetResource<dx12::IndexBuffer>(std::wstring name);
template std::shared_ptr<dx12::IndexBuffer> dx12::ResourceManager::GetResource<dx12::IndexBuffer>(dxhandle_t handle);

#endif//__DX12_INDEXBUFFER_HPP__