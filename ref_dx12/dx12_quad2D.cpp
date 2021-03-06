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

#include "dx12_local.hpp"

dx12::Quad2D::Quad2D()
{
	LOG_FUNC();

	LOG(info) << "Initializing";
	
	//m_context = nullptr;
	//m_vertexBuffer = nullptr;
	//m_indexBuffer = nullptr;
	m_vertexCount = 0;
	m_indexCount = 0;
}
/*
bool dx12::Quad2D::Initialize(ID3D12DeviceContext* context, int x, int y, int width, int height, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();

	if (!context)
	{
		LOG(error) << "Invalid context provided.";
		return false;
	}

	m_context = context;

	if (!InitializeBuffers())
	{
		LOG(error) << "Failed to properly initialize buffers.";
		return false;
	}
	
	LOG(info) << "Successfully initialized 2D subsystem.";

	return UpdateBuffers(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}
*/
bool dx12::Quad2D::InitializeBuffers()
{
	// Courtesy http://www.rastertek.com/dx12tut11.html
	LOG_FUNC();

	HRESULT					hr = E_UNEXPECTED;
	m_vertexCount = m_indexCount = 6;
	Vertex2D*				vertices = new Vertex2D[m_vertexCount];
	unsigned long*			indices = new unsigned long[m_indexCount];
	//D3D12_BUFFER_DESC		vertexBufferDesc, indexBufferDesc;
	D3D12_SUBRESOURCE_DATA	vertexData, indexData;

	if (!vertices)
	{
		LOG(error) << "Failed to allocate memory for vertex buffer.";
		return false;
	}

	if (!indices)
	{
		LOG(error) << "Failed to allocate memory for index buffer.";
		return false;
	}

	// Wipe Structs
	//ZeroMemory(&vertexBufferDesc, sizeof(D3D12_BUFFER_DESC));
	//ZeroMemory(&indexBufferDesc, sizeof(D3D12_BUFFER_DESC));
	ZeroMemory(&vertexData, sizeof(D3D12_SUBRESOURCE_DATA));
	ZeroMemory(&indexData, sizeof(D3D12_SUBRESOURCE_DATA));
	ZeroMemory(vertices, sizeof(Vertex2D) * m_vertexCount);
	ZeroMemory(indices, sizeof(unsigned long) * m_indexCount);

	// Load the index array with data.
	for (unsigned long i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the vertex buffer.
	/*vertexBufferDesc.Usage = D3D12_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex2D) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D12_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D12_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	hr = ref->sys->dx->m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to create vertex buffer.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created vertex buffer.";
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D12_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D12_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = ref->sys->dx->m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to create index buffer.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created index buffer.";
	}
	*/

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	if (vertices)
	{
		delete[] vertices;
		vertices = nullptr;
	}

	if (indices)
	{
		delete[] indices;
		indices = nullptr;
	}

	LOG(info) << "Successfully initialized buffers.";

	return true;
}

bool dx12::Quad2D::UpdateBuffers(int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();

	float						left = 0.0,
								right = 0.0,
								top = 0.0,
								bottom = 0.0;
	Vertex2D*					vertices = nullptr;
	Vertex2D*					verticesPtr = nullptr;
	//D3D12_MAPPED_SUBRESOURCE	mappedResource;
	//ZeroMemory(&mappedResource, sizeof(D3D12_MAPPED_SUBRESOURCE));
	HRESULT						hr = E_UNEXPECTED;
	/*
	if (!m_context)
	{
		LOG(error) << "Invalid context.";
		return false;
	}*/



	// If the parameters have not changed then don't update the vertex buffer since it is currently correct.
	if ((x == m_xPrev) && 
		(y == m_yPrev) &&
		(width == m_widthPrev) &&
		(height == m_heightPrev) &&
		(DirectX::XMVector4Equal(color, m_colorPrev)) &&
		(u1 == m_u1Prev) &&
		(v1 == m_v1Prev) && 
		(u2 == m_u2Prev) && 
		(v2 == m_v2Prev))
	{
		return true;
	}
	
	// If we got this far, the buffer needs to be updated
	m_xPrev = x;
	m_yPrev = y;
	m_widthPrev = width;
	m_heightPrev = height;
	m_colorPrev = color;
	m_u1Prev = u1;
	m_v1Prev = v1;
	m_u2Prev = u2;
	m_v2Prev = v2;

	// Calculate the screen coordinates of the left side of the overlay.
	left = static_cast<float>(((static_cast<float>(ref->sys->dx->m_windowWidth) / 2.0f) * -1.0f) + static_cast<float>(x));

	// Calculate the screen coordinates of the right side of the overlay.
	right = left + static_cast<float>(width);

	// Calculate the screen coordinates of the top of the overlay.
	top = static_cast<float>((static_cast<float>(ref->sys->dx->m_windowHeight) / 2.0f) - static_cast<float>(y));

	// Calculate the screen coordinates of the bottom of the overlay.
	bottom = top - static_cast<float>(height);

	// Create the vertex array.
	vertices = new Vertex2D[m_vertexCount];
	if (!vertices)
	{
		LOG(error) << "Failed to allocate memory for vertex buffer.";
		return false;
	}

	//ZeroMemory(&vertices, sizeof(Vertex2D) * m_vertexCount);

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = DirectX::XMFLOAT4A(left, top, 0.0f, 0.0f);  // Top left.
	vertices[0].color	 = color;
	vertices[0].texCoord = DirectX::XMFLOAT2A(u1, v1);

	vertices[1].position = DirectX::XMFLOAT4A(right, bottom, 0.0f, 0.0f);  // Bottom right.
	vertices[1].color	 = color;
	vertices[1].texCoord = DirectX::XMFLOAT2A(u2, v2);

	vertices[2].position = DirectX::XMFLOAT4A(left, bottom, 0.0f, 0.0f);  // Bottom left.
	vertices[2].color	 = color;
	vertices[2].texCoord = DirectX::XMFLOAT2A(u1, v2);

	// Second triangle.
	vertices[3].position = DirectX::XMFLOAT4A(left, top, 0.0f, 0.0f);  // Top left.
	vertices[3].color	 = color;
	vertices[3].texCoord = DirectX::XMFLOAT2A(u1, v1);

	vertices[4].position = DirectX::XMFLOAT4A(right, top, 0.0f, 0.0f);  // Top right.
	vertices[4].color	 = color;
	vertices[4].texCoord = DirectX::XMFLOAT2A(u2, v1);

	vertices[5].position = DirectX::XMFLOAT4A(right, bottom, 0.0f, 0.0f);  // Bottom right.
	vertices[5].color	 = color;
	vertices[5].texCoord = DirectX::XMFLOAT2A(u2, v2);

	/*
	// Lock the vertex buffer so it can be written to.
	hr = m_context->Map(m_vertexBuffer, 0, D3D12_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to lock vertex buffer.";
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = static_cast<Vertex2D*>(mappedResource.pData);

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, static_cast<void*>(vertices), (sizeof(Vertex2D) * m_vertexCount));

	// Unlock the vertex buffer.
	m_context->Unmap(m_vertexBuffer, 0);
	*/
	// Release the vertex array as it is no longer needed.
	if (vertices)
	{
		delete[] vertices;
		vertices = nullptr;
	}

#ifdef _DEBUG
	DumpD3DDebugMessagesToLog();
#endif

	return true;
}

void dx12::Quad2D::RenderBuffers() const
{
	LOG_FUNC();
	/*
	if (m_context)
	{
		UINT stride = sizeof(Vertex2D);
		UINT offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		m_context->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#ifdef _DEBUG
		DumpD3DDebugMessagesToLog();
#endif
	}
	*/
}

void dx12::Quad2D::Render(int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();
	/*
	if (m_context)
	{
		// Render 2D overlay to back buffer
		if (UpdateBuffers(x, y, width, height, u1, v1, u2, v2, color))
		{
			RenderBuffers();
		}
		else
		{
			LOG(error) << "Failed to update buffers.";
		}
	}
	*/
}

void dx12::Quad2D::Render(int x, int y, int width, int height, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	Render(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}

void dx12::Quad2D::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	//SAFE_RELEASE(m_indexBuffer);

	//SAFE_RELEASE(m_vertexBuffer);

	LOG(info) << "Shutdown complete.";
}