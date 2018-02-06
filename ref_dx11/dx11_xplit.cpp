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

#include "dx11_local.hpp"

using namespace DirectX;

// https://sourceforge.net/p/quake2xp/code/HEAD/tree/trunk/ref_gl/r_light.c
std::vector<dx11::Light> dx11::xpLit::Load(std::string fileName) 
{
	LOG_FUNC();

	int		style, numLights = 0, filter, shadow, ambient, flare, flag, fog;
	XMVECTOR	angles, speed, color, origin, lOrigin, fOrg, radius;
	char	*c, *token, key[256], *value, target[MAX_QPATH];
	float	cone, fSize, fogDensity;
	char	name[MAX_QPATH], path[MAX_QPATH];

	std::vector<dx11::Light>	lights;
	bool						runParseLoop = true;
	std::string					key = "",
								token = "";

	FS_StripExtension(r_worldmodel->name, name, sizeof(name));
	Com_sprintf(path, sizeof(path), "%s.xplit", name);
	FS_LoadFile(path, (void **)&c);

	if (!c) {
		Load_BspLights();
		return;
	}

	LOG(info) << "Loaded lights from " << path;

	while (runParseLoop) {
		token = COM_Parse(&c);

		if (!c)
		{
			runParseLoop = false;
			break;
		}



		Light newLight;

		newLight.m_style = 0;
		newLight.m_filter = 0;
		newLight.m_shadow = 1;
		newLight.m_ambient = 0;
		newLight.m_cone = 0;
		newLight.m_flareSize = 0;
		newLight.m_flare = 0;
		newLight.m_flags = 0;
		newLight.m_fog = 0;
		newLight.m_fogDensity = 0.0;
		memset(target, 0, sizeof(target));
		newLight.m_radius = XMVectorZero();
		newLight.m_angles = XMVectorZero();
		newLight.m_speed = XMVectorZero();
		newLight.m_origin = XMVectorZero();
		newLight.m_lOrigin = XMVectorZero();
		newLight.m_color = XMVectorZero();
		newLight.m_flareOrigin = XMVectorZero();

		while (runParseLoop) 
		{
			token = COM_Parse(&c);
			if (token[0] == '}')
			{
				break;
			}

			strncpy(key, token, sizeof(key) - 1);

			value = COM_Parse(&c);

			if (!_stricmp(key, "radius"))
				sscanf(value, "%f %f %f", &newLight.m_radius.m128_f32[0], &newLight.m_radius.m128_f32[1], &newLight.m_radius.m128_f32[2]);
			else if (!_stricmp(key, "origin"))
				sscanf(value, "%f %f %f", &newLight.m_origin.m128_f32[0], &newLight.m_origin.m128_f32[1], &newLight.m_origin.m128_f32[2]);
			else if (!_stricmp(key, "color"))
				sscanf(value, "%f %f %f", &newLight.m_color.m128_f32[0], &newLight.m_color.m128_f32[1], &newLight.m_color.m128_f32[2]);
			else if (!_stricmp(key, "style"))
				newLight.m_style = atoi(value);
			else if (!_stricmp(key, "filter"))
				newLight.m_filter = atoi(value);
			else if (!_stricmp(key, "angles"))
				sscanf(value, "%f %f %f", &newLight.m_angles.m128_f32[0], &newLight.m_angles.m128_f32[1], &newLight.m_angles.m128_f32[2]);
			else if (!_stricmp(key, "speed"))
				sscanf(value, "%f %f %f", &newLight.m_speed.m128_f32[0], &newLight.m_speed.m128_f32[1], &newLight.m_speed.m128_f32[2]);
			else if (!_stricmp(key, "shadow"))
				newLight.m_shadowCaster = static_cast<bool>(atoi(value));
			else if (!_stricmp(key, "ambient"))
				newLight.m_ambient = atoi(value);
			else if (!_stricmp(key, "_cone"))
				newLight.m_cone = atof(value);
			else if (!_stricmp(key, "flare"))
				newLight.m_flare = static_cast<bool>(atoi(value));
			else if (!_stricmp(key, "flareOrigin"))
				sscanf(value, "%f %f %f", &newLight.m_flareOrigin.m128_f32[0], &newLight.m_flareOrigin.m128_f32[1], &newLight.m_flareOrigin.m128_f32[2]);
			else if (!_stricmp(key, "flareSize"))
				newLight.m_flareSize = atof(value);
			else if (!_stricmp(key, "targetname"))
				Q_strncpyz(target, value, sizeof(target));
			else if (!_stricmp(key, "spawnflags"))
				newLight.m_flags = atoi(value);
			else if (!_stricmp(key, "fogLight"))
				newLight.m_fog = atof(value);
			else if (!_stricmp(key, "fogDensity"))
				newLight.m_fogDensity = atof(value);

		}

		lights.push_back(newLight);
	}
	LOG(info) << "Loaded " << std::to_string(lights.size()) << " lights.";

	return lights;
}
