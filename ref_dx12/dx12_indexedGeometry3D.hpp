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

#ifndef __DX12_INDEXEDGEOMETRY3D_HPP__
#define __DX12_INDEXEDGEOMETRY3D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	__declspec(align(16)) class IndexedGeometry3D {
		friend class System;
		friend class Subsystem3D;
		friend class BSP;
		friend class BSP38;
		friend class BottomLevelAccelerationStructure;

	private:
		std::shared_ptr<VertexBuffer>	m_vertexBuffer = nullptr;
		std::shared_ptr<IndexBuffer>	m_indexBuffer = nullptr;

		std::wstring                    m_uuid7 = L"";

	protected:
		bool                        CreateBuffers(Vertex3D* vertices, size_t vertexBufferSize, unsigned long* indices, size_t indexBufferSize);

		bool                        UpdateVertexBuffer(std::shared_ptr<CommandList> commandList, Vertex3D* vertices, size_t bufferSize);

		bool                        UpdateIndexBuffer(std::shared_ptr<CommandList> commandList, unsigned long* indices, size_t bufferSize);

		void						RenderBuffers(std::shared_ptr<CommandList> commandList) const;

	public:
		IndexedGeometry3D();
		~IndexedGeometry3D();

		virtual void				Render(std::shared_ptr<CommandList> commandList);

		virtual size_t		        VertexCount() { if (m_vertexBuffer) { return m_vertexBuffer->VertexCount(); } return 0; };
		virtual size_t       		IndexCount() { if (m_indexBuffer) { return m_indexBuffer->IndexCount(); } return 0; };

		virtual void				Shutdown();

		std::wstring                GetUUID() { return m_uuid7; };

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_INDEXEDGEOMETRY3D_HPP__