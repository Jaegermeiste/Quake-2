/*
Copyright (C) 2004-2013 Quake2xp Team.

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

#include "dx12_local.hpp"

// Courtesy https://stackoverflow.com/questions/5184988/should-i-use-urldownloadtofile
bool dx12::xpLit::DownloadXPLitForMap(std::string mapName)
{
	LOG_FUNC();

	std::string gameDir = dx12::ref->client->FS_Gamedir();
	std::string mapsPath = dx12::ref->cvars->xpLitPathBaseQ2->String();

	if (gameDir.find("xatrix"))
	{
		mapsPath = dx12::ref->cvars->xpLitPathXatrix->String();
	}
	else if (gameDir.find("rogue"))
	{
		mapsPath = dx12::ref->cvars->xpLitPathRogue->String();
	}

	std::string downloadURL = dx12::ref->cvars->xpLitDownloadPath->String() + mapsPath + mapName + ".xplit?format=raw";

	LOG(info) << "Downloading xpLit for map " << mapName << " from " << downloadURL;

	std::string destinationPath = dx12::ref->sys->GetCurrentWorkingDirectory() + mapsPath + mapName + ".xplit";

	LOG(info) << "Saving xpLit to " << destinationPath;

	return dx12::ref->sys->web->DownloadFile(downloadURL, destinationPath);
}

// https://sourceforge.net/p/quake2xp/code/HEAD/tree/trunk/ref_gl/r_light.c
std::vector<dx12::Light> dx12::xpLit::Load(std::string mapName) 
{
	LOG_FUNC();

	char	*xpLitCharData, *token, key[256], *value, target[MAX_QPATH];

	std::vector<dx12::Light>	lights;
	bool						runParseLoop = false;
	std::string fileName = mapName + ".xplit";

	// Attempt to load relight file from file system
	unsigned int*	xpLitBuffer = nullptr;
	int fileLen = ref->client->FS_LoadFile(fileName, reinterpret_cast<void**>(&xpLitCharData));
	if ((fileLen < 1) || (!xpLitCharData))
	{
		// File not found
		if (ref->cvars->xpLitDownloadEnable->Bool())
		{
			// Download relight file
			if (DownloadXPLitForMap(mapName))
			{
				// Try to load again
				fileLen = ref->client->FS_LoadFile(fileName, reinterpret_cast<void**>(&xpLitBuffer));
			}
		}
	}

	if ((fileLen > 0) && (xpLitBuffer))
	{
		// We successfully loaded the lights file
		LOG(info) << "Loaded lights from " << fileName;
		runParseLoop = true;
	}

	while (runParseLoop) 
	{
		token = COM_Parse(&xpLitCharData);

		if (!xpLitCharData)
		{
			runParseLoop = false;
			break;
		}

		Light newLight;

		memset(target, 0, sizeof(target));

		while (runParseLoop) 
		{
			token = COM_Parse(&xpLitCharData);
			if (token[0] == '}')
			{
				break;
			}

			strncpy_s(key, token, sizeof(key) - 1);

			value = COM_Parse(&xpLitCharData);

			if (!_stricmp(key, "radius"))
				sscanf_s(value, "%f %f %f", &newLight.m_radius.m128_f32[0], &newLight.m_radius.m128_f32[1], &newLight.m_radius.m128_f32[2]);
			if (!_stricmp(key, "origin"))
				sscanf_s(value, "%f %f %f", &newLight.m_origin.m128_f32[0], &newLight.m_origin.m128_f32[1], &newLight.m_origin.m128_f32[2]);
			if (!_stricmp(key, "color"))
				sscanf_s(value, "%f %f %f", &newLight.m_color.m128_f32[0], &newLight.m_color.m128_f32[1], &newLight.m_color.m128_f32[2]);
			if (!_stricmp(key, "style"))
				newLight.m_style = atoi(value);
			if (!_stricmp(key, "filter"))
				newLight.m_filter = atoi(value);
			if (!_stricmp(key, "angles"))
				sscanf_s(value, "%f %f %f", &newLight.m_angles.m128_f32[0], &newLight.m_angles.m128_f32[1], &newLight.m_angles.m128_f32[2]);
			if (!_stricmp(key, "speed"))
				sscanf_s(value, "%f %f %f", &newLight.m_speed.m128_f32[0], &newLight.m_speed.m128_f32[1], &newLight.m_speed.m128_f32[2]);
			if (!_stricmp(key, "shadow"))
				newLight.m_shadowCaster = static_cast<bool>(atoi(value));
			if (!_stricmp(key, "ambient"))
				newLight.m_ambient = atoi(value);
			if (!_stricmp(key, "_cone"))
				newLight.m_cone = atof(value);
			if (!_stricmp(key, "flare"))
				newLight.m_flare = static_cast<bool>(atoi(value));
			if (!_stricmp(key, "flareOrigin"))
				sscanf_s(value, "%f %f %f", &newLight.m_flareOrigin.m128_f32[0], &newLight.m_flareOrigin.m128_f32[1], &newLight.m_flareOrigin.m128_f32[2]);
			if (!_stricmp(key, "flareSize"))
				newLight.m_flareSize = atof(value);
			if (!_stricmp(key, "targetname"))
				strncpy_s(target, value, sizeof(target));
			if (!_stricmp(key, "spawnflags"))
				newLight.m_flags = atoi(value);
			if (!_stricmp(key, "fogLight"))
				newLight.m_fog = atof(value);
			if (!_stricmp(key, "fogDensity"))
				newLight.m_fogDensity = atof(value);

		}

		lights.push_back(newLight);
	}
	LOG(info) << "Loaded " << std::to_string(lights.size()) << " lights.";

	return lights;
}
