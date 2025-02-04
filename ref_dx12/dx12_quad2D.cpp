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

	m_xPrev = -1;
	m_yPrev = -1;
	m_widthPrev = -1;
	m_heightPrev = -1;
	m_u1Prev = -1;
	m_v1Prev = -1;
	m_u2Prev = -1;
	m_v2Prev = -1;
}

bool dx12::Quad2D::Initialize(int x, int y, int width, int height, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();

	LOG(info) << "Initializing.";

	if (!InitializeBuffers(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color))
	{
		LOG(error) << "Failed to properly initialize buffers.";
		return false;
	}
	
	LOG(info) << "Successfully initialized Quad2D.";

	return true; //UpdateBuffers(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}

bool dx12::Quad2D::InitializeBuffers()
{
	return InitializeBuffers(-1, -1, 1, 1, 0.0f, 0.0f, 1.0f, 1.0f, DirectX::Colors::White);
}

bool dx12::Quad2D::InitializeBuffers(int x, int y, int width, int height, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();

	// Calculate the screen coordinates of the left side of the overlay.
	float left = static_cast<float>(x); //static_cast<float>(((static_cast<float>(ref->sys->dx->m_windowWidth) / 2.0f) * -1.0f) + static_cast<float>(x));

	// Calculate the screen coordinates of the right side of the overlay.
	float right = left + static_cast<float>(width);

	// Calculate the screen coordinates of the top of the overlay.
	float top = static_cast<float>(y); //static_cast<float>((static_cast<float>(ref->sys->dx->m_windowHeight) / 2.0f) - static_cast<float>(y));

	// Calculate the screen coordinates of the bottom of the overlay.
	float bottom = top + static_cast<float>(height);

	Vertex2D				vertices[] = {
											{ {  left,  top,    0.0f, 1.0f }, color, { u1, v1 } },   // Top - left
											{ {  right, top,    0.0f, 1.0f }, color, { u2, v1 } },   // Top - right
											{ {  left,  bottom, 0.0f, 1.0f }, color, { u1, v2 } },   // Bottom - left
											{ {  right, bottom, 0.0f, 1.0f }, color, { u2, v2 } }    // Bottom - right
										 };

	unsigned long			indices[] = {
											0, 1, 2,
											2, 1, 3
										};

	if (CreateBuffers(vertices, sizeof(vertices), indices, sizeof(indices)))
	{
		LOG(info) << "Successfully initialized buffers.";

		return true;
	}

	LOG(error) << "Failed to initialize buffers.";

	return false;
}

bool dx12::Quad2D::UpdateBuffers(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();

	float						left = 0.0,
								right = 0.0,
								top = 0.0,
								bottom = 0.0;
	Vertex2D					vertices[4] = {};

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
	left = static_cast<float>(x); //static_cast<float>(((static_cast<float>(ref->sys->dx->m_windowWidth) / 2.0f) * -1.0f) + static_cast<float>(x));

	// Calculate the screen coordinates of the right side of the overlay.
	right = left + static_cast<float>(width);

	// Calculate the screen coordinates of the top of the overlay.
	top = static_cast<float>(y); //static_cast<float>((static_cast<float>(ref->sys->dx->m_windowHeight) / 2.0f) - static_cast<float>(y));

	// Calculate the screen coordinates of the bottom of the overlay.
	bottom = top + static_cast<float>(height);

	ZeroMemory(&vertices, sizeof(Vertex2D) * VertexCount());

	// Load the vertex array with data.
	vertices[0].position = DirectX::XMFLOAT4A(left, top, 0.0f, 1.0f);  // Top left.
	vertices[0].color	 = color;
	vertices[0].texCoord = DirectX::XMFLOAT2A(u1, v1);

	vertices[1].position = DirectX::XMFLOAT4A(right, top, 0.0f, 1.0f);  // Top right.
	vertices[1].color	 = color;
	vertices[1].texCoord = DirectX::XMFLOAT2A(u2, v1);

	vertices[2].position = DirectX::XMFLOAT4A(left, bottom, 0.0f, 1.0f);  // Bottom left.
	vertices[2].color	 = color;
	vertices[2].texCoord = DirectX::XMFLOAT2A(u1, v2);

	vertices[3].position = DirectX::XMFLOAT4A(right, bottom, 0.0f, 1.0f);  // Bottom right.
	vertices[3].color	 = color;
	vertices[3].texCoord = DirectX::XMFLOAT2A(u2, v2);

	return UpdateVertexBuffer(commandList, vertices, sizeof(vertices));
}

void dx12::Quad2D::Render(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	LOG_FUNC();
	
	if (commandList)
	{
		// Render 2D overlay to back buffer
		if (UpdateBuffers(commandList, x, y, width, height, u1, v1, u2, v2, color))
		{
			RenderBuffers(commandList);
		}
		else
		{
			LOG(error) << "Failed to update buffers.";
		}
	}
}

void dx12::Quad2D::Render(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, DirectX::XMVECTORF32 color = DirectX::Colors::White)
{
	Render(commandList, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}