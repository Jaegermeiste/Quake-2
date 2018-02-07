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
	m_header = reinterpret_cast<dheader_t *>(buffer);

	m_version = msl::utilities::SafeInt<unsigned int>(LittleLong(m_header->version));
	if (m_version != BSP38_VERSION)
	{
		ref->client->Sys_Error(ERR_DROP, "BSP " + name + " has wrong version number (" + std::to_string(m_version) + " should be " + std::to_string(BSP38_VERSION) + ").");
		m_header = nullptr;
		return;
	}

	LoadLighting();
}

// Courtesy https://sourceforge.net/p/quake2xp/code/HEAD/tree/trunk/ref_gl/r_light.c
std::vector<dx11::Light> dx11::BSP38::LoadLighting()
{
	LOG_FUNC();

	std::vector<dx11::Light> lights;
	bool addLight = false;
	char *bspEntityString[MAX_MAP_ENTSTRING], *token, key[256], *value, target[MAX_QPATH];
	bool runTokenLoop = true;

	if (!m_header)
	{
		ref->client->Con_Printf(PRINT_ALL, "No map loaded.");
		runTokenLoop = false;
		return lights;
	}

	lump_t* entLump = &m_header->lumps[BSP38_LUMP_ENTITIES];
	if (entLump->filelen > MAX_MAP_ENTSTRING)
	{
		ref->client->Sys_Error(ERR_DROP, "Map has too large entity lump: " + std::to_string(entLump->filelen));
		return lights;
	}

	memcpy(bspEntityString, reinterpret_cast<byte*>(m_header) + entLump->fileofs, msl::utilities::SafeInt<size_t>(entLump->filelen));

	while (runTokenLoop)
	{
		token = COM_Parse(bspEntityString);
		if (!bspEntityString)
			break;

		Light newLight;

		memset(target, 0, sizeof(target));

		addLight = false;

		while (runTokenLoop) {
			token = COM_Parse(bspEntityString);
			if (token[0] == '}')
				break;

			strncpy(key, token, sizeof(key) - 1);

			value = COM_Parse(bspEntityString);
			if (!_stricmp(key, "classname")) 
			{
				if (!_stricmp(value, "light"))
				{
					addLight = true;
				}
				if (!_stricmp(value, "light_mine1")) 
				{
					addLight = true;
				}
				if (!_stricmp(value, "light_mine2")) 
				{
					addLight = true;
				}
			}

			if (!_stricmp(key, "light"))
				newLight.m_radius.m128_f32[0] = atof(value);
			if (!_stricmp(key, "origin"))
				sscanf(value, "%f %f %f", &newLight.m_origin.m128_f32[0], &newLight.m_origin.m128_f32[1], &newLight.m_origin.m128_f32[2]);
			if (!_stricmp(key, "color"))
				sscanf(value, "%f %f %f", &newLight.m_color.m128_f32[0], &newLight.m_color.m128_f32[1], &newLight.m_color.m128_f32[2]);
			if (!_stricmp(key, "style"))
				newLight.m_style = atoi(value);
			if (!_stricmp(key, "_cone"))
				newLight.m_cone = atof(value);
			if (!_stricmp(key, "spawnflags"))
				newLight.m_flags = atoi(value);
		}

		if (addLight)
		{
			lights.push_back(newLight);
		}
	}
	LOG(info) << "Loaded " << std::to_string(lights.size()) << " lights.";

	return lights;
}

dx11::BSP38::~BSP38()
{

}