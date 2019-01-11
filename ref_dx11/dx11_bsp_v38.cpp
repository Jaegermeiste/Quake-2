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

typedef vec3_t BSP38_d_vertex;

dx11::BSP38::BSP38(std::string name, unsigned int* buffer)
{
	m_name = name;
	m_version = BSP38_VERSION;
	m_header = reinterpret_cast<bsp_disk_header *>(buffer);

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

			strncpy_s(key, token, sizeof(key) - 1);

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
				sscanf_s(value, "%f %f %f", &newLight.m_origin.m128_f32[0], &newLight.m_origin.m128_f32[1], &newLight.m_origin.m128_f32[2]);
			if (!_stricmp(key, "color"))
				sscanf_s(value, "%f %f %f", &newLight.m_color.m128_f32[0], &newLight.m_color.m128_f32[1], &newLight.m_color.m128_f32[2]);
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

unsigned int dx11::BSP38::LoadVertices(bsp_disk_header* header)
{
	unsigned int	i = 0;
	
	// Clear any existing data in memory
	if (d_vertices != nullptr)
	{
		delete[] d_vertices;
		d_vertices = nullptr;
	}
	d_numVertices = 0;

	//
	// Load Disk Vertices
	//
	size_t d_vertexDataLength = header->lumps[BSP38_LUMP_VERTEXES].filelen;
	d_numVertices = d_vertexDataLength / sizeof(float[3]);

	if (d_vertexDataLength % sizeof(float[3]))
	{
		ref->client->Sys_Error(ERR_DROP, "Unexpected lump size in LUMP_VERTICES.");
		return 0;
	}

	float* vertexInputArray = reinterpret_cast<float*>(*(&header + header->lumps[BSP38_LUMP_VERTEXES].fileofs));
	d_vertices = new DirectX::XMFLOAT3A[d_numVertices];

	for (i = 0; i < d_numVertices; i += 3)
	{
		d_vertices[i].x = LittleFloat(vertexInputArray[i + 0]);
		d_vertices[i].y = LittleFloat(vertexInputArray[i + 1]);
		d_vertices[i].z = LittleFloat(vertexInputArray[i + 2]);
	}

	return d_numVertices;
}

unsigned int dx11::BSP38::LoadTexInfo(bsp_disk_header* header)
{
	unsigned int	i = 0,
					j = 0;

	// Clear any existing data in memory
	if (d_texInfo != nullptr)
	{
		delete[] d_texInfo;
		d_texInfo = nullptr;
	}
	d_numTexInfo = 0;

	//
	// Load Disk TexInfo
	//
	size_t				d_texInfoDataLength = header->lumps[BSP38_LUMP_TEXINFO].filelen;
	d_numTexInfo		= d_texInfoDataLength / sizeof(float[3]);

	if (d_texInfoDataLength % sizeof(texinfo_t))
	{
		ref->client->Sys_Error(ERR_DROP, "Unexpected lump size in LUMP_TEXINFO.");
		return 0;
	}

	texinfo_t*				texInfoInputArray	= reinterpret_cast<texinfo_t*>(*(&header + header->lumps[BSP38_LUMP_TEXINFO].fileofs));
	texinfo_t*				d_texInfo			= new texinfo_t[d_numTexInfo];

	for (i = 0; i < d_numTexInfo; i++)
	{
		for (j = 0; j < 8; j++)
		{
			d_texInfo[i].vecs[0][j] = LittleFloat(texInfoInputArray[i].vecs[0][j]);
		}

		d_texInfo[i].flags = LittleLong(texInfoInputArray[i].flags);
		
		// FIXME: Do we actually need next?
		/*next = LittleLong(texInfoInputArray[i].nexttexinfo);
		if (next > 0)
			next = loadmodel->texinfo + next;
		else
			d_texInfo[i].next = NULL;*/

		// Load texture
		/*Com_sprintf(name, sizeof(name), "textures/%s.wal", texInfoInputArray[i].texture);
		d_texInfo[i].image = GL_FindImage(name, it_wall);
		if (!d_texInfo[i].image)
		{
			ri.Con_Printf(PRINT_ALL, "Couldn't load %s\n", name);
			d_texInfo[i].image = r_notexture;
		}
		*/

		// u = x * u_axis.x + y * u_axis.y + z * u_axis.z + u_offset
		// v = x * v_axis.x + y * v_axis.y + z * v_axis.z + v_offset

		float u = x * d_texInfo[i].vecs[0][0] + y * d_texInfo[i].vecs[0][1] + z * d_texInfo[i].vecs[0][2] + d_texInfo[i].vecs[0][3];
		float v = x * d_texInfo[i].vecs[1][0] + y * d_texInfo[i].vecs[1][1] + z * d_texInfo[i].vecs[1][2] + d_texInfo[i].vecs[1][3];

		
	}

	return m_numVertices;
}