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
ref_dx11
2017 Bleeding Eye Studios
*/

#include "dx11_local.hpp"

dx11::Subsystem2D::Subsystem2D()
{
	BOOST_LOG_NAMED_SCOPE("Subsystem2D");

	LOG(info) << "Initializing";

	m_2DdeferredContext = nullptr;
	m_2DcommandList = nullptr;
	m_2DrenderTargetTexture = nullptr;
	m_2DoverlayRTV = nullptr;
	m_2DshaderResourceView = nullptr;
	ZeroMemory(&m_2DorthographicMatrix, sizeof(DirectX::XMMATRIX));
	m_depthDisabledStencilState = nullptr;

	m_2DvertexBuffer = nullptr;
	m_2DindexBuffer = nullptr;
	m_2DvertexCount = 0;
	m_2DindexCount = 0;
}

/*
http://rastertek.com/dx11tut11.html
*/
bool dx11::Subsystem2D::Initialize()
{
	BOOST_LOG_NAMED_SCOPE("Subsystem2D::Initialize");

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;

	// Wipe Structs
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&hr, sizeof(HRESULT));
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	ZeroMemory(&depthDisabledStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	// Set modified for first run
	ref->cvars->overlayScale->SetModified(true);

	// Set width and height
	m_renderTargetWidth = ref->sys->m_windowWidth;
	m_renderTargetHeight = ref->sys->m_windowHeight;

	// Create context
	hr = ref->sys->m_d3dDevice->CreateDeferredContext(0, &m_2DdeferredContext);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create 2D deferred context.";
		return false;
	}

	// Setup the render target texture description.
	textureDesc.Width = m_renderTargetWidth;
	textureDesc.Height = m_renderTargetHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	hr = ref->sys->m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_2DrenderTargetTexture);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create Texture2D.";
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	hr = ref->sys->m_d3dDevice->CreateRenderTargetView(m_2DrenderTargetTexture, &renderTargetViewDesc, &m_2DoverlayRTV);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create RenderTargetView.";
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = ref->sys->m_d3dDevice->CreateShaderResourceView(m_2DrenderTargetTexture, &shaderResourceViewDesc, &m_2DshaderResourceView);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create ShaderResourceView.";
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_2DdeferredContext->OMSetRenderTargets(1, &m_2DoverlayRTV, nullptr);

	// Create an orthographic projection matrix for 2D rendering.
	m_2DorthographicMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(m_renderTargetWidth), static_cast<float>(m_renderTargetHeight), ref->cvars->zNear2D->Float(), ref->cvars->zFar2D->Float());

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	hr = ref->sys->m_d3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create DepthStencilState.";
		return false;
	}

	if (!InitializeBuffers())
	{
		LOG(error) << "Failed to properly initialize buffers.";
		return false;
	}

	if (!m_2Dshader.Initialize(ref->sys->m_d3dDevice, "simpleVertex.vs", "simplePixel.ps"))
	{
		LOG(error) << "Failed to properly create shaders.";
		return false;
	}

	LOG(info) << "Successfully initialized 2D subsystem.";

	return true;
}

bool dx11::Subsystem2D::InitializeBuffers()
{
	// Courtesy http://www.rastertek.com/dx11tut11.html
	BOOST_LOG_NAMED_SCOPE("Subsystem2D::InitializeBuffers");

	HRESULT hr;
	m_2DvertexCount = m_2DindexCount = 6;
	Vertex2D* vertices = new Vertex2D[m_2DvertexCount];
	unsigned long* indices = new unsigned long[m_2DindexCount];
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

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
	ZeroMemory(&hr, sizeof(HRESULT));
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(vertices, sizeof(Vertex2D) * m_2DvertexCount);

	// Load the index array with data.
	for (unsigned long i = 0; i < m_2DindexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex2D) * m_2DvertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	hr = ref->sys->m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_2DvertexBuffer);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to create vertex buffer.";
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_2DindexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = ref->sys->m_d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_2DindexBuffer);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to create index buffer.";
		return false;
	}

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

void dx11::Subsystem2D::Clear()
{
	BOOST_LOG_NAMED_SCOPE("Subsystem2D::Clear");

	// Clear 2D deferred
	if (m_2DdeferredContext)
	{
		LOG(trace) << "Clearing overlay RenderTargetView.";

		// Clear the GUI Overlay buffer to transparent
		m_2DdeferredContext->ClearRenderTargetView(m_2DoverlayRTV, DirectX::Colors::Transparent);
	}
}

bool dx11::Subsystem2D::UpdateBuffers()
{
	float						left			= 0.0, 
								right			= 0.0, 
								top				= 0.0,
								bottom			= 0.0;
	Vertex2D*					vertices		= nullptr;
	Vertex2D*					verticesPtr		= nullptr;
	D3D11_MAPPED_SUBRESOURCE	mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT						hr				= E_UNEXPECTED;

	// Don't update the vertex buffer if the scale hasn't changed (positions are the same)
	if (!ref->cvars->overlayScale->Modified())
	{
		return true;
	}

	// Calculate the screen coordinates of the left side of the overlay.
	left = static_cast<float>((ref->sys->m_windowWidth / 2) * -1.0) + static_cast<float>((m_renderTargetWidth * ref->cvars->overlayScale->Float()) / 2.0);

	// Calculate the screen coordinates of the right side of the overlay.
	right = left + static_cast<float>(m_renderTargetWidth * ref->cvars->overlayScale->Float());

	// Calculate the screen coordinates of the top of the overlay.
	top = static_cast<float>(ref->sys->m_windowHeight / 2) - static_cast<float>((m_renderTargetHeight * ref->cvars->overlayScale->Float()) / 2.0);

	// Calculate the screen coordinates of the bottom of the overlay.
	bottom = top - static_cast<float>(m_renderTargetHeight * ref->cvars->overlayScale->Float());

	LOG(info) << "Calculated overlay coordinates: left=" << left << " right=" << right << " top=" << top << " bottom=" << bottom;

	// Create the vertex array.
	vertices = new Vertex2D[m_2DvertexCount];
	if (!vertices)
	{
		LOG(error) << "Failed to allocate memory for vertex buffer.";
		return false;
	}

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texCoord = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texCoord = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texCoord = DirectX::XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texCoord = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texCoord = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texCoord = DirectX::XMFLOAT2(1.0f, 1.0f);

	// Lock the vertex buffer so it can be written to.
	hr = m_2DdeferredContext->Map(m_2DvertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		LOG(error) << "Failed to lock vertex buffer.";
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (Vertex2D*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(Vertex2D) * m_2DvertexCount));

	// Unlock the vertex buffer.
	m_2DdeferredContext->Unmap(m_2DvertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	if (vertices)
	{
		delete[] vertices;
		vertices = nullptr;
	}

	return true;
}

void dx11::Subsystem2D::RenderBuffers()
{
	if (m_2DdeferredContext)
	{
		unsigned int stride = sizeof(Vertex2D);
		unsigned int offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		m_2DdeferredContext->IASetVertexBuffers(0, 1, &m_2DvertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		m_2DdeferredContext->IASetIndexBuffer(m_2DindexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		m_2DdeferredContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void dx11::Subsystem2D::Render()
{
	BOOST_LOG_NAMED_SCOPE("Subsystem2D::Render");

	ref->sys->m_immediateContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);

	if (m_2DcommandList)
	{
		ref->sys->m_immediateContext->ExecuteCommandList(m_2DcommandList, TRUE);
	}
}

void dx11::Subsystem2D::Shutdown()
{
	BOOST_LOG_NAMED_SCOPE("Subsystem2D::Shutdown");

	LOG(info) << "Shutting down.";

	SAFE_RELEASE(m_2DindexBuffer);

	SAFE_RELEASE(m_2DvertexBuffer);

	SAFE_RELEASE(m_depthDisabledStencilState);

	SAFE_RELEASE(m_2DdeferredContext);

	SAFE_RELEASE(m_2DshaderResourceView);

	SAFE_RELEASE(m_2DoverlayRTV);

	SAFE_RELEASE(m_2DrenderTargetTexture);

	LOG(info) << "Shutdown complete.";
}
