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

#ifndef __DX12_MAP_HPP__
#define __DX12_MAP_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Map
	{
	private:
		std::wstring				m_mapName;

		std::unique_ptr<BSP>		m_bsp = nullptr;

		std::vector<Light>			m_lights;

	public:
									Map();

		bool						Initialize();
		void						Shutdown();

		void						Load(std::wstring mapName);
	};
}

#endif // !__DX12_MAP_HPP__