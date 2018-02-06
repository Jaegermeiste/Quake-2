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

dx11::BSP38::BSP38(std::string name, unsigned int* buffer)
{
	m_name = name;
	m_version = BSP38_VERSION;

	dheader_t	*header = reinterpret_cast<dheader_t *>(buffer);

	m_version = LittleULong(header->version);
	if (m_version != BSP38_VERSION)
	{
		ref->client->Sys_Error(ERR_DROP, "BSP " + name + " has wrong version number (" + std::to_string(m_version) + " should be " + std::to_string(BSP38_VERSION) + ").");
		return;
	}

	LoadLighting();
}

// Courtesy https://stackoverflow.com/questions/5184988/should-i-use-urldownloadtofile
bool dx11::BSP38::DownloadXPLitForMap(std::string mapName)
{
	LOG_FUNC();

	std::string gameDir = dx11::ref->client->FS_Gamedir();
	std::string mapsPath = dx11::ref->cvars->xpLitPathBaseQ2->String();

	if (gameDir.find("xatrix"))
	{
		mapsPath = dx11::ref->cvars->xpLitPathXatrix->String();
	}
	else if (gameDir.find("rogue"))
	{
		mapsPath = dx11::ref->cvars->xpLitPathRogue->String();
	}

	std::string downloadURL = dx11::ref->cvars->xpLitDownloadPath->String() + mapsPath + mapName + ".xplit?format=raw";

	LOG(info) << "Downloading xpLit for map " << mapName << " from " << downloadURL;

	std::string destinationPath = dx11::ref->sys->GetCurrentWorkingDirectory() + mapsPath + mapName + ".xplit";

	LOG(info) << "Saving xpLit to " << destinationPath;

	return dx11::ref->sys->web->DownloadFile(downloadURL, destinationPath);
}

void dx11::BSP38::LoadLighting()
{
	// Attempt to load relight file from file system
	unsigned int*	xpLitBuffer = nullptr;
	int fileLen = ref->client->FS_LoadFile(m_name, reinterpret_cast<void**>(&xpLitBuffer));
	if ((fileLen < 1) || (!xpLitBuffer))
	{
		// File not found
		if (ref->cvars->xpLitDownloadEnable->Bool())
		{
			// Download relight file
			if (DownloadXPLitForMap(m_name))
			{
				// Try to load again
				fileLen = ref->client->FS_LoadFile(m_name, reinterpret_cast<void**>(&xpLitBuffer));

				if ((fileLen < 1) || (!xpLitBuffer))
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	if ((fileLen > 0) && (xpLitBuffer))
	{
		// We sucessfully loaded the lights file
	}
}

dx11::BSP38::~BSP38()
{

}