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

#ifndef __DX12_VERTEXBUFFER_HPP__
#define __DX12_VERTEXBUFFER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class VertexBuffer : public ResourceBuffer
	{
		friend class ResourceManager;
	public:
		VertexBuffer(std::wstring name) : ResourceBuffer(name)
		{
			m_type = RESOURCE_VERTEXBUFFER;
			m_count = 0;
		};

		bool CreateVertexBuffer(Vertex2D* bufferData, size_t bufferSize) {
			return CreateBuffer(bufferData, bufferSize);
		}

		bool UpdateVertexBuffer(std::shared_ptr<CommandList> commandList, Vertex2D* bufferData, size_t bufferSize) {
			return UpdateBuffer(commandList, bufferData, bufferSize);
		}

		unsigned int		VertexCount() { return m_count; };
	};
}

template bool dx12::ResourceBuffer::CreateBuffer<dx12::Vertex2D>(dx12::Vertex2D* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UploadBuffer<dx12::Vertex2D>(dx12::Vertex2D* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UpdateBuffer<dx12::Vertex2D>(std::shared_ptr<CommandList> commandList, dx12::Vertex2D* bufferData, size_t bufferSize);

template std::shared_ptr<dx12::VertexBuffer> dx12::ResourceManager::CreateResource<dx12::VertexBuffer>(std::wstring name);
template std::shared_ptr<dx12::VertexBuffer> dx12::ResourceManager::GetOrCreateResource<dx12::VertexBuffer>(std::wstring name);
template std::shared_ptr<dx12::VertexBuffer> dx12::ResourceManager::GetResource<dx12::VertexBuffer>(std::wstring name);
template std::shared_ptr<dx12::VertexBuffer> dx12::ResourceManager::GetResource<dx12::VertexBuffer>(dxhandle_t handle);

#endif//__DX12_VERTEXBUFFER_HPP__