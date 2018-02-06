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

dx11::Map::Map()
{
	LOG_FUNC();
}

bool dx11::Map::Initialize()
{
	LOG_FUNC();

	return true;
}

void dx11::Map::Shutdown()
{
	LOG_FUNC();
}

void dx11::Map::Load(std::string mapName)
{
	LOG_FUNC();

	unsigned int*	fileBuffer = nullptr;

	if (mapName.empty())
	{
		ref->client->Sys_Error(ERR_DROP, "Empty filename provided.");
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
	if (!fileBuffer)
	{
		ref->client->Sys_Error(ERR_DROP, "Map " + mapName + " not found.");
		return;
	}

	// call the apropriate loader
	if (LittleULong(*fileBuffer) == IDBSPHEADER)
	{
		disk_bsp_header_t* header = reinterpret_cast<disk_bsp_header_t *>(fileBuffer);

		int bspVersion = LittleLong(header->version);

		switch (bspVersion)
		{
		case BSP38_VERSION:
			// Quake 2
			m_bsp = std::make_unique<BSP38>(mapName, fileBuffer);
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
		ref->client->Sys_Error(ERR_DROP, "Unknown file type for " + mapName);
	}

	ref->client->FS_FreeFile(fileBuffer);
}
