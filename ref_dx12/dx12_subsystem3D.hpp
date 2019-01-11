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

#ifndef __DX12_3D_HPP__
#define __DX12_3D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	__declspec(align(16)) class Subsystem3D {
		friend class System;
	private:
		DirectX::XMMATRIX			m_3DworldMatrix;
		DirectX::XMMATRIX			m_3DviewMatrix;
		DirectX::XMMATRIX			m_3DprojectionMatrix;

	public:
									Subsystem3D();

		bool						Initialize();

		void						Clear();

		void						Render();

		void						Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_3D_HPP__