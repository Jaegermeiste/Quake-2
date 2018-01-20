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

// Courtesy https://stackoverflow.com/questions/5184988/should-i-use-urldownloadtofile


bool DownloadXPLitForMap(std::string mapName)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
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

	std::string destinationPath = dx11::GetCurrentWorkingDirectory() + mapsPath + mapName + ".xplit";

	LOG(info) << "Saving xpLit to " << destinationPath;

	hr = URLDownloadToFile(NULL, downloadURL.c_str(), destinationPath.c_str(), 0, NULL);

	if (hr == E_OUTOFMEMORY) {
		LOG(error) << "Download Failed: Buffer length invalid, or insufficient memory";
		return false;
	}
	else if (hr == INET_E_DOWNLOAD_FAILURE) {
		LOG(error) << "Download Failed: URL is invalid";
		return false;
	}
	else if (FAILED(hr)) {
		LOG(error) << "Other error: " << hr;
		return false;
	}

	return true;
}