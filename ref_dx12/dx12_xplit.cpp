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
bool dx12::xpLit::DownloadXPLitForMap(std::wstring mapName)
{
	LOG_FUNC();

	try
	{
		std::wstring gameDir = dx12::ref->client->FS_Gamedir();
		std::wstring mapsPath = dx12::ref->cvars->xpLitPathBaseQ2->String();

		if (gameDir.find(L"xatrix"))
		{
			mapsPath = dx12::ref->cvars->xpLitPathXatrix->String();
		}
		else if (gameDir.find(L"rogue"))
		{
			mapsPath = dx12::ref->cvars->xpLitPathRogue->String();
		}

		std::wstring downloadURL = dx12::ref->cvars->xpLitDownloadPath->String() + mapsPath + mapName + L".xplit?format=raw";

		LOG(info) << "Downloading xpLit for map " << mapName << " from " << downloadURL;

		std::wstring destinationPath = dx12::ref->sys->GetCurrentWorkingDirectory() + mapsPath + mapName + L".xplit";

		LOG(info) << "Saving xpLit to " << destinationPath;

		return dx12::ref->sys->web->DownloadFile(downloadURL, destinationPath);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

// https://sourceforge.net/p/quake2xp/code/HEAD/tree/trunk/ref_gl/r_light.c
std::vector<dx12::Light> dx12::xpLit::Load(std::wstring mapName)
{
	LOG_FUNC();

	std::vector<dx12::Light>	lights = {};

	try
	{
		char* xpLitCharData = nullptr, * token = nullptr, key[256] = {}, * value = nullptr, target[MAX_QPATH] = {};


		bool						runParseLoop = false;
		std::wstring fileName = mapName + L".xplit";

		// Attempt to load relight file from file system
		unsigned int* xpLitBuffer = nullptr;
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

			Light newLight({ 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f);

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
				{
					float radiusX = 1.0f, radiusY = 1.0f, radiusZ = 1.0f;
					sscanf_s(value, "%f %f %f", &radiusX, &radiusY, &radiusZ);
					float meanRadius = (radiusX + radiusY + radiusZ) / 3;
					newLight.linearAttenuation = 4.5f / meanRadius;
					newLight.quadraticAttenuation = 75.0f / (meanRadius * meanRadius);
				}
				if (!_stricmp(key, "origin"))
					sscanf_s(value, "%f %f %f", &newLight.origin.m128_f32[0], &newLight.origin.m128_f32[1], &newLight.origin.m128_f32[2]);
				if (!_stricmp(key, "color"))
					sscanf_s(value, "%f %f %f", &newLight.color.m128_f32[0], &newLight.color.m128_f32[1], &newLight.color.m128_f32[2]);
				if (!_stricmp(key, "style"))
					newLight.style = atoi(value);
				if (!_stricmp(key, "filter"))
					newLight.filter = atoi(value);
				if (!_stricmp(key, "angles"))
					sscanf_s(value, "%f %f %f", &newLight.angles.m128_f32[0], &newLight.angles.m128_f32[1], &newLight.angles.m128_f32[2]);
				if (!_stricmp(key, "speed"))
					sscanf_s(value, "%f %f %f", &newLight.speed.m128_f32[0], &newLight.speed.m128_f32[1], &newLight.speed.m128_f32[2]);
				if (!_stricmp(key, "shadow"))
					newLight.shadowCaster = static_cast<bool>(atoi(value));
				if (!_stricmp(key, "ambient"))
					newLight.ambient = atoi(value);
				if (!_stricmp(key, "_cone"))
					newLight.cone = atof(value);
				if (!_stricmp(key, "flare"))
					newLight.flare = static_cast<bool>(atoi(value));
				if (!_stricmp(key, "flareOrigin"))
					sscanf_s(value, "%f %f %f", &newLight.flareOrigin.m128_f32[0], &newLight.flareOrigin.m128_f32[1], &newLight.flareOrigin.m128_f32[2]);
				if (!_stricmp(key, "flareSize"))
					newLight.flareSize = atof(value);
				if (!_stricmp(key, "targetname"))
					strncpy_s(target, value, sizeof(target));
				if (!_stricmp(key, "spawnflags"))
					newLight.flags = atoi(value);
				if (!_stricmp(key, "fogLight"))
					newLight.fog = atof(value);
				if (!_stricmp(key, "fogDensity"))
					newLight.fogDensity = atof(value);

			}

			lights.push_back(newLight);
		}
		LOG(info) << "Loaded " << std::to_string(lights.size()) << " lights.";

	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return lights;
}
