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

unsigned int dx11::BSP::LoadDiskVertices_v29_v38(void* data, unsigned int offset, size_t length)
{
	// inputs are float[3]
	float*			inputArray	= reinterpret_cast<float*>(*(&data + offset));
	unsigned int	i			= 0, 
					count		= length / sizeof(float[3]);

	// Clear any existing data
	if (m_vertices != nullptr)
	{
		delete[] m_vertices;
		m_vertices = nullptr;
	}
	m_numVertices = 0;

	if (length % sizeof(float[3]))
	{
		ref->client->Sys_Error(ERR_DROP, "Unexpected lump size.");
		return 0;
	}

	m_vertices = new DirectX::XMFLOAT3A[count];

	for (i = 0; i < count; i += 3)
	{
		m_vertices[i].x = LittleFloat(inputArray[i + 0]);
		m_vertices[i].y = LittleFloat(inputArray[i + 1]);
		m_vertices[i].z = LittleFloat(inputArray[i + 2]);
	}

	m_numVertices = count;

	return count;
}