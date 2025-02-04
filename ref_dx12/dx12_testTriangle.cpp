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

dx12::TestTriangle::TestTriangle()
{
	LOG_FUNC();

	m_aspectRatio = 1.0f;
}

bool dx12::TestTriangle::Initialize(int width, int height)
{
	LOG_FUNC();

	LOG(info) << "Initializing.";

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	if (!InitializeBuffers())
	{
		LOG(error) << "Failed to properly initialize buffers.";
		return false;
	}
	
	LOG(info) << "Successfully initialized TestTriangle.";
	return true;
}

bool dx12::TestTriangle::InitializeBuffers()
{
	LOG_FUNC();

	Vertex2D				vertices[] = {
											{ {  0.0f,   0.25f * m_aspectRatio, 0.0f, 0.0f }, DirectX::Colors::Red,   { 0.0f, 0.0f } },
											{ {  0.25f, -0.25f * m_aspectRatio, 0.0f, 0.0f }, DirectX::Colors::Green, { 0.0f, 0.0f } },
											{ { -0.25f, -0.25f * m_aspectRatio, 0.0f, 0.0f }, DirectX::Colors::Blue,  { 0.0f, 0.0f } }
	};

	unsigned long			indices[] = {
											0, 1, 2
	};

	if (CreateBuffers(vertices, sizeof(vertices), indices, sizeof(indices)))
	{
		LOG(info) << "Successfully initialized buffers.";

		return true;
	}

	LOG(error) << "Failed to initialize buffers.";

	return false;
}