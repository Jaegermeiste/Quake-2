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

#include "dx12_local.hpp"

dx12::IndexedGeometry2D::IndexedGeometry2D()
{
	LOG_FUNC();

	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_uuid7 = L"";
}

dx12::IndexedGeometry2D::~IndexedGeometry2D()
{
	LOG_FUNC();

	Shutdown();
}

bool dx12::IndexedGeometry2D::CreateBuffers(Vertex2D* vertices, size_t vertexBufferSize, unsigned long* indices, size_t indexBufferSize)
{
	LOG_FUNC();

	try {
		m_uuid7 = GetUUIDv7();

		m_vertexBuffer = ref->res->CreateResource<VertexBuffer>(L"Vertex_" + m_uuid7);

		if (!m_vertexBuffer)
		{
			LOG(error) << "Failed to obtain vertex buffer resource!";
			return false;
		}

		if (!m_vertexBuffer->CreateVertexBuffer(vertices, vertexBufferSize))
		{
			LOG(error) << "Failed to create vertex buffer!";
			return false;
		}

		m_indexBuffer = ref->res->CreateResource<IndexBuffer>(L"Index_" + m_uuid7);

		if (!m_indexBuffer)
		{
			LOG(error) << "Failed to obtain index buffer resource!";
			return false;
		}

		if (!m_indexBuffer->CreateIndexBuffer(indices, indexBufferSize))
		{
			LOG(error) << "Failed to create index buffer!";
			return false;
		}

		return true;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::IndexedGeometry2D::UpdateVertexBuffer(std::shared_ptr<CommandList> commandList, Vertex2D* vertices, size_t bufferSize) {
	LOG_FUNC();

	try {
		if (m_vertexBuffer) {
			return m_vertexBuffer->UpdateVertexBuffer(commandList, vertices, bufferSize);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::IndexedGeometry2D::UpdateIndexBuffer(std::shared_ptr<CommandList> commandList, unsigned long* indices, size_t bufferSize) {
	LOG_FUNC();

	try {
		if (m_indexBuffer) {
			return m_indexBuffer->UpdateIndexBuffer(commandList, indices, bufferSize);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

void dx12::IndexedGeometry2D::RenderBuffers(std::shared_ptr<CommandList> commandList) const
{
	LOG_FUNC();

	try {
		if (commandList && commandList->IsOpen())
		{
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
			vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			vertexBufferView.StrideInBytes = sizeof(Vertex2D);
			vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(Vertex2D) * m_vertexBuffer->VertexCount());

			D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
			indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
			indexBufferView.Format = DXGI_FORMAT_R32_UINT;
			indexBufferView.SizeInBytes = static_cast<UINT>(sizeof(unsigned long) * m_indexBuffer->IndexCount());

			// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
			commandList->List()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Set the vertex buffer to active in the input assembler so it can be rendered.
			commandList->List()->IASetVertexBuffers(0, 1, &vertexBufferView);

			// Set the index buffer to active in the input assembler so it can be rendered.
			commandList->List()->IASetIndexBuffer(&indexBufferView);

			commandList->List()->DrawIndexedInstanced(static_cast<UINT>(m_indexBuffer->IndexCount()), 1, 0, 0, 0);

#ifdef _DEBUG
			DumpD3DDebugMessagesToLog();
#endif
		}
		else
		{
			LOG(warning) << "RenderBuffers called on closed or null CommandList!";
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::IndexedGeometry2D::Render(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		if (commandList)
		{
			RenderBuffers(commandList);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::IndexedGeometry2D::Shutdown()
{
	try
	{
		if (m_indexBuffer) {
			SAFE_RELEASE(m_indexBuffer);
		}

		if (m_vertexBuffer) {
			SAFE_RELEASE(m_vertexBuffer);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}