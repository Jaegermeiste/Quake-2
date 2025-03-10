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

dx12::Map::Map()
{
	LOG_FUNC();
}

bool dx12::Map::Initialize()
{
	LOG_FUNC();

	return true;
}

void dx12::Map::Shutdown()
{
	LOG_FUNC();
}

void dx12::Map::Load(std::wstring mapName)
{
	LOG_FUNC();

	try
	{
		unsigned int* fileBuffer = nullptr;

		if (mapName.empty())
		{
			ref->client->Sys_Error(ERR_DROP, L"Empty filename provided.");
		}

		if (mapName == m_mapName)
		{
			// This is the currently loaded map
			return;
		}

		//
		// load the file
		//
		int fileLen = ref->client->FS_LoadFile(mapName, reinterpret_cast<void**>(&fileBuffer));
		if ((fileLen < 1) || (!fileBuffer))
		{
			ref->client->Sys_Error(ERR_DROP, L"Map " + mapName + L" not found.");
			return;
		}

		// call the appropriate loader
		if (LittleULong(*fileBuffer) == IDBSPHEADER)
		{
			disk_bsp_header_s* header = reinterpret_cast<disk_bsp_header_s*>(fileBuffer);

			int bspVersion = LittleLong(header->version);

			switch (bspVersion)
			{
			case BSP38_VERSION:
				// Quake 2
				m_bsp = std::make_unique<BSP38>(mapName, fileBuffer);

				// Lighting
				/*m_lights = xpLit::Load(mapName);

				// If we got nothing back from xpLit, load from the BSP
				if ((m_lights.size() == 0) && (m_bsp))
				{
					m_bsp->LoadLighting();
				}*/
				//m_bsp->Upload(ref->sys->dx->subsystem3D->CommandList());

				break;
			case BSP46_VERSION:
				// Quake 3
				break;
			case BSP47_VERSION:
				// RTCW
				break;
			default:
				break;
			}
		}
		else if (LittleULong(*fileBuffer) == 29)
		{
			// Quake 1 BSP
		}
		else
		{
			ref->client->Sys_Error(ERR_DROP, L"Unknown file type for " + mapName);
		}

		ref->client->FS_FreeFile(fileBuffer);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
