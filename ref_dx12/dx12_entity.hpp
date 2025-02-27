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

#ifndef __DX12_ENTITY_HPP__
#define __DX12_ENTITY_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Entity
	{
	private:
		XMFLOAT4A	m_origin;
		XMMATRIX    m_rotation;
		XMVECTOR	m_color;

		dxhandle_t  m_modelHandle = 0;
		dxhandle_t  m_skinHandle = 0;

	public:
		Entity(XMFLOAT4A origin, XMMATRIX rotation, XMVECTOR color = DirectX::Colors::White, dxhandle_t modelHandle = 0, dxhandle_t skinHandle = 0)
		{
			m_origin = origin;
			m_rotation = rotation;
			m_color = color;
			m_modelHandle = modelHandle;
			m_skinHandle = skinHandle;
		};

		Entity(vec3_t origin, vec3_t viewAngles, XMVECTOR color = DirectX::Colors::White, dxhandle_t modelHandle = 0, dxhandle_t skinHandle = 0)
		{
			m_origin = { origin[0], origin[1], origin[2], 1.0f };
			m_rotation = XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(viewAngles[PITCH]),  // Pitch
				XMConvertToRadians(viewAngles[YAW]),  // Yaw
				XMConvertToRadians(viewAngles[ROLL])   // Roll
			);
			m_color = color;
			m_modelHandle = modelHandle;
			m_skinHandle = skinHandle;
		};

		XMFLOAT4A	Origin() { return m_origin; };
		XMMATRIX    Rotation() { return m_rotation; };
		XMVECTOR	Color() { return m_color; };

		dxhandle_t  ModelHandle() { return m_modelHandle; };
		dxhandle_t  SkinHandle() { return m_skinHandle; };
	};
}

#endif // !__DX12_ENTITY_HPP__