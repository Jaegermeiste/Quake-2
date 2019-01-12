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

dx12::SubsystemText::SubsystemText()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

#ifdef USE_DIRECT2D
	m_writeFactory = nullptr;
	m_textFormat = nullptr;
#endif
}

bool dx12::SubsystemText::Initialize()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

#ifdef USE_DIRECT2D
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_writeFactory)
	);

	if (FAILED(hr))
	{
		LOG(error) << "Failed to create write factory.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created write factory.";
	}

	hr = m_writeFactory->CreateTextFormat(
		ref->sys->ToWideString(ref->cvars->font->String()).c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		9,
		L"en-us",
		&m_textFormat
	);

	if (FAILED(hr))
	{
		LOG(error) << "Failed to create text format.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created text format.";
	}

	// Center the text horizontally and vertically.
	m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
#endif

	return true;
}

bool dx12::SubsystemText::InitializeBuffers()
{
	// Courtesy http://www.rastertek.com/dx12tut11.html
	LOG_FUNC();

	HRESULT					hr = E_UNEXPECTED;

	//D3D12_BUFFER_DESC		vertexBufferDesc, indexBufferDesc;
	D3D12_SUBRESOURCE_DATA	vertexData, indexData;

	m_vertices = new Vertex2D[MAX_VERTICES]();
	m_indices = new unsigned long[MAX_INDICES]();
	m_vertexCount = m_indexCount = 0;

	if (!m_vertices)
	{
		LOG(error) << "Failed to allocate memory for vertex buffer.";
		return false;
	}

	if (!m_indices)
	{
		LOG(error) << "Failed to allocate memory for index buffer.";
		return false;
	}

	// Wipe Structs
	//ZeroMemory(&vertexBufferDesc, sizeof(D3D12_BUFFER_DESC));
	//ZeroMemory(&indexBufferDesc, sizeof(D3D12_BUFFER_DESC));
	ZeroMemory(&vertexData, sizeof(D3D12_SUBRESOURCE_DATA));
	ZeroMemory(&indexData, sizeof(D3D12_SUBRESOURCE_DATA));

	// Set up the description of the vertex buffer.
	/*vertexBufferDesc.Usage = D3D12_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex2D) * MAX_VERTICES;
	vertexBufferDesc.BindFlags = D3D12_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D12_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
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
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * MAX_INDICES;
	indexBufferDesc.BindFlags = D3D12_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = m_indices;
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

	LOG(info) << "Successfully initialized buffers.";
	*/
	return true;
}

void dx12::SubsystemText::DrawSmallChar(int x, int y, int ch)
{
	LOG_FUNC();
}

void dx12::SubsystemText::DrawSmallStringExt(int x, int y, std::string string, const DirectX::XMVECTORF32 setColor, bool forceColor)
{
	LOG_FUNC();
}

void dx12::SubsystemText::DrawBigChar(int x, int y, int ch)
{
	LOG_FUNC();
}

void dx12::SubsystemText::DrawBigStringExt(int x, int y, std::string string, const DirectX::XMVECTORF32 setColor, bool forceColor)
{
	LOG_FUNC();
}

void dx12::SubsystemText::Flush()
{
	LOG_FUNC();

	/*if ((m_vertexCount > 0) && (m_indexCount > 0) && (m_context))
	{
		static UINT stride = sizeof(Vertex2D);
		static UINT offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		m_context->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (m_shader->Render(m_context, m_indexCount, DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->media->img->m_conChars->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer))
		{
			// Render succeeded, so reset
			m_inBatch = false;
			m_vertexCount = 0;
			m_indexCount = 0;
		}

#ifdef _DEBUG
		DumpD3DDebugMessagesToLog();
#endif
	}
	*/
}

#ifdef USE_DIRECT2D
void dx12::SubsystemText::RenderText(int x, int y, int w, int h, WCHAR* text, ID2D1SolidColorBrush* colorBrush)
{
	LOG_FUNC();
	
	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->BeginDraw();

	//ref->sys->dx->m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	//ref->sys->dx->m_d2dContext->DrawText(
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->DrawText(
		text,
		wcslen(text),
		m_textFormat,
		D2D1::RectF(x, y, x + w, y + h),
		colorBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->EndDraw();
	
}

void dx12::SubsystemText::RenderText(int x, int y, int w, int h, std::string text, ID2D1SolidColorBrush* colorBrush)
{
	LOG_FUNC();

	ref->sys->dx->subsystem2D->ActivateD2DDrawing();
	
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->BeginDraw();

	//ref->sys->dx->m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	
	//ref->sys->dx->m_d2dContext->DrawText(
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->DrawText(
		ref->sys->ToWideString(text).c_str(),
		text.length(),
		m_textFormat,
		D2D1::RectF(x, y,x + w, y + h),
		colorBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->EndDraw();

}
#endif

void dx12::SubsystemText::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

#ifdef USE_DIRECT2D
	SAFE_RELEASE(m_textFormat);

	SAFE_RELEASE(m_writeFactory);
#endif

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = nullptr;
	}

	if (m_indices)
	{
		delete[] m_indices;
		m_indices = nullptr;
	}

	LOG(info) << "Shutdown complete.";
}
