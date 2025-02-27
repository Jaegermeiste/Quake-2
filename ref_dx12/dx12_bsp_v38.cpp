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
2025 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

typedef vec3_t BSP38_d_vertex;



dx12::BSP38::BSP38(std::wstring name, unsigned int* buffer)
{
	LOG_FUNC();

	try
	{
		m_name = name;
		m_version = BSP38_VERSION;
		m_header = reinterpret_cast<bsp_disk_header*>(buffer);

		m_version = msl::utilities::SafeInt<unsigned int>(LittleLong(m_header->version));
		if (m_version != BSP38_VERSION)
		{
			ref->client->Sys_Error(ERR_DROP, L"BSP " + name + L" has wrong version number (" + std::to_wstring(m_version) + L" should be " + std::to_wstring(BSP38_VERSION) + L").");
			m_header = nullptr;
			return;
		}

		m_header38 = reinterpret_cast<bsp38_disk_header*>(buffer);

		// swap all the lumps
		for (unsigned int i = 0; i < sizeof(dheader_t) / 4; i++)
		{
			((int*)m_header38)[i] = LittleLong(((int*)m_header38)[i]);
		}

		m_diskCompressedVis = {};

		// Load from disk
		LoadVertices();
		LoadEdges();
		LoadSurfEdges();
		LoadLighting();
		LoadPlanes();
		LoadTexInfo();
		LoadFaces();
		LoadLeafFaces();
		LoadVisibility();
		LoadLeafs();
		LoadNodes();
		LoadSubModels();

		LOG(info) << "BSP38 " << m_name << " loaded.";
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::BSP38::~BSP38()
{
	try
	{
		Shutdown();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

size_t dx12::BSP38::LoadPlanes()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0, j = 0;

		// Clear any existing data in memory
		if (!m_diskPlanes.empty())
		{
			m_diskPlanes.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_PLANES].filelen;

		size_t numPlanes = dataLength / sizeof(bsp38_plane_t);

		if (dataLength % sizeof(bsp38_plane_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_PLANES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<bsp38_plane_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_PLANES].fileofs);

		m_diskPlanes.resize(numPlanes);

		for (i = 0; i < numPlanes; i++)
		{
			for (j = 0; j < 3; j++)
			{
				m_diskPlanes[i].normal[j] = LittleFloat(inputArray[i].normal[j]);
			}

			m_diskPlanes[i].dist = LittleFloat(inputArray[i].dist);
			m_diskPlanes[i].type = LittleLong(inputArray[i].type);
		}

		LOG(info) << "Loaded " << m_diskPlanes.size() << "/" << numPlanes << " BSP38 bsp38_plane_t.";

		return numPlanes;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadVertices()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0;

		// Clear any existing data in memory
		if (!m_diskVertices.empty())
		{
			m_diskVertices.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_VERTEXES].filelen;

		size_t numVertices = dataLength / sizeof(float[3]);

		if (dataLength % sizeof(float[3]))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_VERTEXES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<float*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_VERTEXES].fileofs);

		m_diskVertices.resize(numVertices);

		for (i = 0; i < numVertices; i += 3)
		{
			m_diskVertices[i].x = LittleFloat(inputArray[i + 0]);
			m_diskVertices[i].y = LittleFloat(inputArray[i + 1]);
			m_diskVertices[i].z = LittleFloat(inputArray[i + 2]);
		}

		LOG(info) << "Loaded " << m_diskVertices.size() << "/" << numVertices << " BSP38 vertices.";

		return numVertices;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadEdges()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0;

		// Clear any existing data in memory
		if (!m_diskEdges.empty())
		{
			m_diskEdges.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_EDGES].filelen;

		size_t numEdges = dataLength / sizeof(dedge_t);

		if (dataLength % sizeof(dedge_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_EDGES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<dedge_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_EDGES].fileofs);

		m_diskEdges.resize(numEdges);

		for (i = 0; i < numEdges; i++)
		{
			m_diskEdges[i].v[0] = LittleShort(inputArray[i].v[0]);
			m_diskEdges[i].v[1] = LittleShort(inputArray[i].v[1]);
		}

		LOG(info) << "Loaded " << m_diskEdges.size() << "/" << numEdges << " BSP38 bsp38_edge_t.";

		return numEdges;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadSurfEdges()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0;

		// Clear any existing data in memory
		if (!m_diskSurfEdges.empty())
		{
			m_diskSurfEdges.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_SURFEDGES].filelen;

		size_t numSurfEdges = dataLength / sizeof(int);

		if (dataLength % sizeof(int))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_SURFEDGES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<int*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_SURFEDGES].fileofs);

		m_diskSurfEdges.resize(numSurfEdges);

		for (i = 0; i < numSurfEdges; i++)
		{
			m_diskSurfEdges[i] = LittleLong(inputArray[i]);
		}

		LOG(info) << "Loaded " << m_diskSurfEdges.size() << "/" << numSurfEdges << " BSP38 surfedges (int).";

		return numSurfEdges;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadTexInfo()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0,
			j = 0;

		// Clear any existing data in memory
		if (!m_diskTexInfo.empty())
		{
			m_diskTexInfo.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_TEXINFO].filelen;

		size_t numTexInfo = dataLength / sizeof(texinfo_t);

		if (dataLength % sizeof(texinfo_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_TEXINFO lump size.");
			return 0;
		}

		texinfo_t* inputArray = reinterpret_cast<texinfo_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_TEXINFO].fileofs);
		m_diskTexInfo.resize(numTexInfo);

		for (i = 0; i < numTexInfo; i++)
		{
			for (j = 0; j < 8; j++)
			{
				m_diskTexInfo[i].vecs[0][j] = LittleFloat(inputArray[i].vecs[0][j]);
			}

			m_diskTexInfo[i].flags = LittleLong(inputArray[i].flags);

			// Load texture
			std::wstring fileName = L"textures/" + ref->sys->ToWideString(inputArray[i].texture) + L".wal";

			auto image = ref->media->img->Load(fileName, it_wall);

			if (!image)
			{
				ref->client->Con_Printf(PRINT_ALL, L"Couldn't load " + fileName + L"\n");
			}
		}

		LOG(info) << "Loaded " << m_diskTexInfo.size() << "/" << numTexInfo << " BSP38 texinfo_t.";

		return numTexInfo;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

// Courtesy https://sourceforge.net/p/quake2xp/code/HEAD/tree/trunk/ref_gl/r_light.c
size_t dx12::BSP38::LoadLighting()
{
	LOG_FUNC();

	try
	{
		bool addLight = false;
		char* bspEntityString = new char[MAX_MAP_ENTSTRING], * token, key[256], * value, target[MAX_QPATH];
		bool runTokenLoop = true;

		if (!m_header38)
		{
			ref->client->Con_Printf(PRINT_ALL, L"No map loaded.");
			runTokenLoop = false;
			return m_lights.size();
		}

		lump_t* entLump = &m_header38->lumps[BSP38_LUMP_ENTITIES];
		if (entLump->filelen > MAX_MAP_ENTSTRING)
		{
			ref->client->Sys_Error(ERR_DROP, L"Map has too large entity lump: " + std::to_wstring(entLump->filelen));
			return m_lights.size();
		}

		memcpy(bspEntityString, reinterpret_cast<byte*>(m_header38) + entLump->fileofs, msl::utilities::SafeInt<size_t>(entLump->filelen));

		while (runTokenLoop)
		{
			token = COM_Parse(&bspEntityString);
			if (!bspEntityString)
				break;

			Light newLight({ 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f);

			memset(target, 0, sizeof(target));

			addLight = false;

			while (runTokenLoop) {
				token = COM_Parse(&bspEntityString);
				if (token[0] == '}')
					break;

				strncpy_s(key, token, sizeof(key) - 1);

				value = COM_Parse(&bspEntityString);
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
				{
					float radius = atof(value);
					newLight.linearAttenuation = 4.5f / radius;
					newLight.quadraticAttenuation = 75.0f / (radius * radius);
				}
				if (!_stricmp(key, "origin"))
					sscanf_s(value, "%f %f %f", &newLight.origin.m128_f32[0], &newLight.origin.m128_f32[1], &newLight.origin.m128_f32[2]);
				if (!_stricmp(key, "_color"))
					sscanf_s(value, "%f %f %f", &newLight.color.m128_f32[0], &newLight.color.m128_f32[1], &newLight.color.m128_f32[2]);
				if (!_stricmp(key, "style"))
					newLight.style = atoi(value);
				if (!_stricmp(key, "_cone"))
					newLight.cone = atof(value);
				if (!_stricmp(key, "targetname"))
					strncpy_s(target, value, sizeof(target));
				if (!_stricmp(key, "spawnflags"))
					newLight.flags = atoi(value);
			}

			if (addLight)
			{
				m_lights.push_back(newLight);
			}
		}
		LOG(info) << "Loaded " << m_lights.size() << " lights.";

		if (bspEntityString)
		{
			delete[] bspEntityString;
			bspEntityString = nullptr;
		}

		return m_lights.size();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadFaces()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0, j = 0;

		// Clear any existing data in memory
		if (!m_diskFaces.empty())
		{
			m_diskFaces.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_FACES].filelen;

		size_t numFaces = dataLength / sizeof(dface_t);

		if (dataLength % sizeof(dface_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_FACES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<dface_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_FACES].fileofs);

		m_diskFaces.resize(numFaces);

		for (i = 0; i < numFaces; i++)
		{
			m_diskFaces[i].planenum = LittleUShort(inputArray[i].planenum);
			m_diskFaces[i].side = LittleShort(inputArray[i].side);
			m_diskFaces[i].firstedge = LittleLong(inputArray[i].firstedge);
			m_diskFaces[i].numedges = LittleShort(inputArray[i].numedges);
			m_diskFaces[i].texinfo = LittleShort(inputArray[i].texinfo);

			for (j = 0; j < MAXLIGHTMAPS; j++)
				m_diskFaces[i].styles[j] = m_diskFaces[i].styles[j];

			m_diskFaces[i].lightofs = LittleLong(inputArray[i].lightofs);
		}

		LOG(info) << "Loaded " << m_diskFaces.size() << "/" << numFaces << " BSP38 dface_t.";

		return numFaces;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadLeafFaces()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0;

		// Clear any existing data in memory
		if (!m_diskLeafFaces.empty())
		{
			m_diskLeafFaces.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_LEAFFACES].filelen;

		size_t numMarkSurfaces = dataLength / sizeof(short);

		if (dataLength % sizeof(short))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_LEAFFACES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<short*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_LEAFFACES].fileofs);

		m_diskLeafFaces.resize(numMarkSurfaces);

		for (i = 0; i < numMarkSurfaces; i++)
		{
			m_diskLeafFaces[i] = LittleShort(inputArray[i]);
		}

		LOG(info) << "Loaded " << m_diskLeafFaces.size() << "/" << numMarkSurfaces << " BSP38 LeafFaces (short).";

		return numMarkSurfaces;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadVisibility()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0;

		size_t dataLength = m_header38->lumps[BSP38_LUMP_VISIBILITY].filelen;

		auto inputArray = reinterpret_cast<dvis_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_VISIBILITY].fileofs);

		m_diskCompressedVis.numclusters = LittleLong(inputArray->numclusters);

		m_diskCompressedVis.bitofs = new int* [m_diskCompressedVis.numclusters];
		for (i = 0; i < m_diskCompressedVis.numclusters; i++)
		{
			m_diskCompressedVis.bitofs[i] = new int[2];
		}

		for (i = 0; i < m_diskCompressedVis.numclusters; i++)
		{
			m_diskCompressedVis.bitofs[i][DVIS_PVS] = LittleLong(inputArray->bitofs[i][DVIS_PVS]);
			m_diskCompressedVis.bitofs[i][DVIS_PHS] = LittleLong(inputArray->bitofs[i][DVIS_PHS]);
		}

		LOG(info) << "Loaded " << m_diskCompressedVis.numclusters << " BSP38 dvis_t.";

		return m_diskCompressedVis.numclusters;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadLeafs()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0, j = 0;

		// Clear any existing data in memory
		if (!m_diskLeafs.empty())
		{
			m_diskLeafs.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_LEAFS].filelen;

		size_t numLeafs = dataLength / sizeof(dleaf_t);

		if (dataLength % sizeof(dleaf_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_LEAFS lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<dleaf_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_LEAFS].fileofs);

		m_diskLeafs.resize(numLeafs);

		for (i = 0; i < numLeafs; i++)
		{
			m_diskLeafs[i].contents = LittleLong(inputArray[i].contents);

			m_diskLeafs[i].cluster = LittleShort(inputArray[i].cluster);
			m_diskLeafs[i].area = LittleShort(inputArray[i].area);

			for (j = 0; j < 3; j++)
			{
				m_diskLeafs[i].mins[j] = LittleShort(inputArray[i].mins[j]);
				m_diskLeafs[i].maxs[j] = LittleShort(inputArray[i].maxs[j]);
			}

			m_diskLeafs[i].firstleafface = LittleUShort(inputArray[i].firstleafface);
			m_diskLeafs[i].numleaffaces = LittleUShort(inputArray[i].numleaffaces);

			m_diskLeafs[i].firstleafbrush = LittleUShort(inputArray[i].firstleafbrush);
			m_diskLeafs[i].numleafbrushes = LittleUShort(inputArray[i].numleafbrushes);
		}

		LOG(info) << "Loaded " << m_diskLeafs.size() << "/" << numLeafs << " BSP38 dleaf_t.";

		return numLeafs;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadNodes()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0, j = 0;

		// Clear any existing data in memory
		if (!m_diskNodes.empty())
		{
			m_diskNodes.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_NODES].filelen;

		size_t numNodes = dataLength / sizeof(dnode_t);

		if (dataLength % sizeof(dnode_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_NODES lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<dnode_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_NODES].fileofs);

		m_diskNodes.resize(numNodes);

		for (i = 0; i < numNodes; i++)
		{
			m_diskNodes[i].planenum = LittleLong(inputArray[i].planenum);

			for (j = 0; j < 2; j++)
			{
				m_diskNodes[i].children[j] = LittleLong(inputArray[i].children[j]);
			}

			for (j = 0; j < 3; j++)
			{
				m_diskNodes[i].mins[j] = LittleShort(inputArray[i].mins[j]);
				m_diskNodes[i].maxs[j] = LittleShort(inputArray[i].maxs[j]);
			}

			m_diskNodes[i].firstface = LittleUShort(inputArray[i].firstface);
			m_diskNodes[i].numfaces = LittleUShort(inputArray[i].numfaces);
		}

		LOG(info) << "Loaded " << m_diskNodes.size() << "/" << numNodes << " BSP38 dnode_t.";

		return numNodes;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

size_t dx12::BSP38::LoadSubModels()
{
	LOG_FUNC();

	try
	{
		unsigned int	i = 0, j = 0;

		// Clear any existing data in memory
		if (!m_diskSubModels.empty())
		{
			m_diskSubModels.clear();
		}

		size_t dataLength = m_header38->lumps[BSP38_LUMP_MODELS].filelen;

		size_t numSubModels = dataLength / sizeof(dmodel_t);

		if (dataLength % sizeof(dmodel_t))
		{
			ref->client->Sys_Error(ERR_DROP, L"Unexpected BSP38_LUMP_MODELS lump size.");
			return 0;
		}

		auto inputArray = reinterpret_cast<dmodel_t*>(reinterpret_cast<byte*>(m_header38) + m_header38->lumps[BSP38_LUMP_MODELS].fileofs);

		m_diskSubModels.resize(numSubModels);

		for (i = 0; i < numSubModels; i++)
		{
			for (j = 0; j < 3; j++)
			{
				m_diskSubModels[i].mins[j] = LittleFloat(inputArray[i].mins[j]);
				m_diskSubModels[i].maxs[j] = LittleFloat(inputArray[i].maxs[j]);
			}

			for (j = 0; j < 3; j++)
			{
				m_diskSubModels[i].origin[j] = LittleFloat(inputArray[i].origin[j]);
			}

			m_diskSubModels[i].headnode = LittleLong(inputArray[i].headnode);
			m_diskSubModels[i].firstface = LittleLong(inputArray[i].firstface);
			m_diskSubModels[i].numfaces = LittleLong(inputArray[i].numfaces);
		}

		LOG(info) << "Loaded " << m_diskSubModels.size() << "/" << numSubModels << " BSP38 dmodel_t.";

		return numSubModels;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

void dx12::BSP38::ComputeIndices()
{
	try
	{
		for (const auto& face : m_diskFaces)
		{
			for (int i = 0; i < face.numedges; i++)
			{
				int edgeIndex = m_diskSurfEdges[static_cast<size_t>(face.firstedge) + i];

				if (edgeIndex < 0)
				{
					// Negative edge indices mean reversed order
					m_indices.push_back(m_diskEdges[-edgeIndex].v[1]);
					m_indices.push_back(m_diskEdges[-edgeIndex].v[0]);
				}
				else
				{
					m_indices.push_back(m_diskEdges[edgeIndex].v[0]);
					m_indices.push_back(m_diskEdges[edgeIndex].v[1]);
				}
			}
		}

		LOG(info) << "Generated " << m_indices.size() << " indices for BSP38.";
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::Upload(std::shared_ptr<CommandList> commandList)
{
	try
	{
		CollectLeafGeometry();
		UploadLeafGeometry();
		GenerateBottomLevelAS(commandList);
		//GenerateTopLevelAS(commandList);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::CollectLeafGeometry()
{
	LOG_FUNC();

	try
	{
		// Map to hold unique vertices per leaf to avoid duplicates
		struct VertexKey {
			size_t vertexIndex;
			size_t texInfoIndex;

			bool operator==(const VertexKey& other) const {
				return vertexIndex == other.vertexIndex && texInfoIndex == other.texInfoIndex;
			}
		};

		struct VertexKeyHash {
			size_t operator()(const VertexKey& key) const {
				return std::hash<size_t>()(key.vertexIndex) ^ std::hash<size_t>()(key.texInfoIndex);
			}
		};

		// Clear any existing data in memory
		if (!m_leafGeometry.empty())
		{
			m_leafGeometry.clear();
		}

		// Build the array
		m_leafGeometry.resize(m_diskLeafs.size());

		// Collect geometry for each leaf
		for (size_t leafIdx = 0; leafIdx < m_diskLeafs.size(); leafIdx++) {
			const auto leaf = m_diskLeafs[leafIdx];
			auto& leafGeometry = m_leafGeometry[leafIdx];

			leafGeometry.index = leafIdx;
			leafGeometry.cluster = leaf.cluster;
			leafGeometry.area = leaf.area;

			// Map to prevent duplicate vertices within the leaf
			std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;

			// Find index of first LeafFace
			//uint16_t startLeafFaceIndex = 0;
			//auto it = std::find(m_diskLeafFaces.begin(), m_diskLeafFaces.end(), leaf.firstleafface);

			if (leaf.firstleafface < m_diskLeafFaces.size()) {
				//startLeafFaceIndex = std::distance(m_diskLeafFaces.begin(), it);

				for (size_t i = 0; i < leaf.numleaffaces; i++) {
					if ((leaf.firstleafface + i) >= m_diskLeafFaces.size())
					{
						LOG(warning) << "Invalid LeafFace index " << (leaf.firstleafface + i) << " against LeafFaces array of size " << m_diskLeafFaces.size() << ", leaf " << leafIdx << " is degenerate!";
						break;
					}
					uint16_t faceIndex = m_diskLeafFaces[leaf.firstleafface + i];
					const auto face = m_diskFaces[faceIndex];
					const auto texInfo = m_diskTexInfo[face.texinfo];

					// Find index of first surfedge
					int startSurfEdgeIndex = face.firstedge;// 0;
					/*auto it2 = std::find(m_diskSurfEdges.begin(), m_diskSurfEdges.end(), face.firstedge);

					if (it2 != m_diskSurfEdges.end()) {
						startSurfEdgeIndex = std::distance(m_diskSurfEdges.begin(), it2);
					}
					else {
						LOG(error) << "Starting SurfEdge " << face.firstedge << " not found!";
					}*/

					std::vector<uint32_t> faceIndices;
					for (short e = 0; e < face.numedges; e++) {
						int32_t surfEdgeIndex = m_diskSurfEdges[static_cast<size_t>(startSurfEdgeIndex) + e];
						int32_t edgeIndex = abs(surfEdgeIndex);
						const auto edge = m_diskEdges[edgeIndex];

						uint16_t vertexIndex = (surfEdgeIndex >= 0) ? edge.v[0] : edge.v[1];

						// Create a unique key for this vertex and texture combination
						VertexKey vKey = { vertexIndex, static_cast<size_t>(face.texinfo) };

						// Check if we've already added this vertex
						auto it = vertexMap.find(vKey);
						uint32_t mappedIndex;
						if (it != vertexMap.end()) {
							// Vertex already exists
							mappedIndex = it->second;
						}
						else {
							// Create new vertex
							const auto bspVertex = m_diskVertices[vertexIndex];

							Vertex3D vertex = {};

							// Convert positions and adjust for coordinate system (swap y and z, negate z)
							vertex.position = XMFLOAT4A(
								bspVertex.x,
								bspVertex.z,
								-bspVertex.y,
								1.0f
							);

							// Calculate texture coordinates
							float s = texInfo.vecs[0][0] * bspVertex.x + texInfo.vecs[0][1] * bspVertex.y + texInfo.vecs[0][2] * bspVertex.z + texInfo.vecs[0][3];
							float t = texInfo.vecs[1][0] * bspVertex.x + texInfo.vecs[1][1] * bspVertex.y + texInfo.vecs[1][2] * bspVertex.z + texInfo.vecs[1][3];

							// Assume a texture size of 256x256
							float textureWidth = 256.0f;
							float textureHeight = 256.0f;

							auto texName = ref->sys->ToWideString(texInfo.texture);
							if (!texName.empty() && !(std::all_of(texName.begin(), texName.end(), [](auto c) {return std::isspace(c); })))
							{
								// Try load image
								std::wstring fileName = L"textures/" + ref->sys->ToWideString(texInfo.texture) + L".wal";

								auto image = ref->media->img->Load(fileName, it_wall);

								if (image)
								{
									textureWidth = image->GetWidth();
									textureHeight = image->GetHeight();
								}
							}

							vertex.texCoord0 = XMFLOAT2(s / textureWidth, t / textureHeight);
							vertex.texCoord1 = XMFLOAT2(s / textureWidth, t / textureHeight);

							// Initialize normal and tangent to zero; we'll compute later
							vertex.normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
							vertex.tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);

							// Add vertex to leaf's vertex buffer
							mappedIndex = static_cast<uint32_t>(leafGeometry.vertices.size());
							leafGeometry.vertices.push_back(vertex);

							// Map the vertex key to the index
							vertexMap[vKey] = mappedIndex;
						}

						// Add index to face indices
						faceIndices.push_back(mappedIndex);
					}

					// Triangulate the face (assumed to be convex)
					for (size_t j = 1; j < faceIndices.size() - 1; ++j) {
						leafGeometry.indices.push_back(faceIndices[0]);
						leafGeometry.indices.push_back(faceIndices[j]);
						leafGeometry.indices.push_back(faceIndices[j + 1]);

						// Compute normal for the triangle
						Vertex3D& v0 = leafGeometry.vertices[faceIndices[0]];
						Vertex3D& v1 = leafGeometry.vertices[faceIndices[j]];
						Vertex3D& v2 = leafGeometry.vertices[faceIndices[j + 1]];

						XMVECTOR p0 = XMLoadFloat4A(&v0.position);
						XMVECTOR p1 = XMLoadFloat4A(&v1.position);
						XMVECTOR p2 = XMLoadFloat4A(&v2.position);

						XMVECTOR uv0 = XMLoadFloat2(&v0.texCoord0);
						XMVECTOR uv1 = XMLoadFloat2(&v1.texCoord0);
						XMVECTOR uv2 = XMLoadFloat2(&v2.texCoord0);

						XMVECTOR edge1 = XMVectorSubtract(p1, p0);
						XMVECTOR edge2 = XMVectorSubtract(p2, p0);

						XMVECTOR deltaUV1 = XMVectorSubtract(uv1, uv0);
						XMVECTOR deltaUV2 = XMVectorSubtract(uv2, uv0);

						// Calculate normal
						XMVECTOR faceNormal = XMVector3Cross(edge1, edge2);
						faceNormal = XMVector3Normalize(faceNormal);

						// Accumulate normals for smooth shading
						XMFLOAT3 normal;
						XMStoreFloat3(&normal, faceNormal);

						v0.normal.x += normal.x;
						v0.normal.y += normal.y;
						v0.normal.z += normal.z;

						v1.normal.x += normal.x;
						v1.normal.y += normal.y;
						v1.normal.z += normal.z;

						v2.normal.x += normal.x;
						v2.normal.y += normal.y;
						v2.normal.z += normal.z;

						// Calculate tangent
						// Compute f (determinant)
						float du1 = XMVectorGetX(deltaUV1);
						float dv1 = XMVectorGetY(deltaUV1);
						float du2 = XMVectorGetX(deltaUV2);
						float dv2 = XMVectorGetY(deltaUV2);

						float f = 1.0f / (du1 * dv2 - du2 * dv1 + 1e-8f); // Add small epsilon to prevent division by zero

						// Compute tangent
						XMVECTOR tangent = f * (dv2 * edge1 - dv1 * edge2);

						// Accumulate tangents
						v0.tangent = XMFLOAT3(v0.tangent.x + XMVectorGetX(tangent), v0.tangent.y + XMVectorGetY(tangent), v0.tangent.z + XMVectorGetZ(tangent));
						v1.tangent = XMFLOAT3(v1.tangent.x + XMVectorGetX(tangent), v1.tangent.y + XMVectorGetY(tangent), v1.tangent.z + XMVectorGetZ(tangent));
						v2.tangent = XMFLOAT3(v2.tangent.x + XMVectorGetX(tangent), v2.tangent.y + XMVectorGetY(tangent), v2.tangent.z + XMVectorGetZ(tangent));
					}
				}

				// After processing all faces in the leaf
				for (auto& vertex : leafGeometry.vertices) {
					// Normalize the normal
					XMVECTOR normal = XMLoadFloat3(&vertex.normal);
					normal = XMVector3Normalize(normal);
					XMStoreFloat3(&vertex.normal, normal);

					// Normalize the tangent
					XMVECTOR tangent = XMLoadFloat3(&vertex.tangent);
					tangent = XMVector3Normalize(tangent);
					XMStoreFloat3(&vertex.tangent, tangent);
				}
			}
			else {
				LOG(error) << "Starting LeafFace " << leaf.firstleafface << " not found!";
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::UploadLeafGeometry()
{
	LOG_FUNC();

	try
	{
		// Clear any existing data in memory
		if (!m_leafAccelStructures.empty())
		{
			m_leafAccelStructures.clear();
		}

		// Presize the array, but don't allocate
		m_leafAccelStructures.reserve(m_leafGeometry.size());

		for (unsigned int i = 0; i < m_leafGeometry.size(); i++)
		{
			const auto leafGeometry = m_leafGeometry[i];

			if ((leafGeometry.cluster >= 0) && (leafGeometry.area > 0) && (leafGeometry.vertices.size() > 0) && (leafGeometry.indices.size() > 0))
			{
				auto leafAS = m_leafAccelStructures.emplace_back();

				const Vertex3D* constRawVertexArray = leafGeometry.vertices.data();
				Vertex3D* rawVertexArray = const_cast<Vertex3D*>(constRawVertexArray);
				size_t rawVertexBufferSize = leafGeometry.vertices.size() * sizeof(Vertex3D);

				const unsigned long* constRawIndexArray = leafGeometry.indices.data();
				unsigned long* rawIndexArray = const_cast<unsigned long*>(constRawIndexArray);
				size_t rawIndexBufferSize = leafGeometry.indices.size() * sizeof(unsigned long);

				if (!leafAS.CreateGeometryBuffers(rawVertexArray, rawVertexBufferSize, rawIndexArray, rawIndexBufferSize))
				{
					LOG(warning) << "Failed to upload geometry for leaf " << leafGeometry.index;
				}
			}
		}

		LOG(info) << "Uploaded " << m_leafAccelStructures.size() << " valid / " << m_leafGeometry.size() << " total leafs.";
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::GenerateBottomLevelAS(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		for (auto leafAS : m_leafAccelStructures)
		{
			if (!leafAS.CreateBLAS(commandList))
			{
				LOG(warning) << "Failed to create Bottom Level Acceleration Structure " << leafAS.GetUUID();
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::ClearDiskData()
{
	try
	{
		if (m_header38)
		{
			//delete m_header38;
			m_header38 = nullptr;
		}

		m_diskVertices.clear();
		m_diskVertices.shrink_to_fit();

		m_diskEdges.clear();
		m_diskEdges.shrink_to_fit();

		m_diskSurfEdges.clear();
		m_diskSurfEdges.shrink_to_fit();

		//m_lights.clear();
		//m_lights.shrink_to_fit();

		m_diskPlanes.clear();
		m_diskPlanes.shrink_to_fit();

		m_diskTexInfo.clear();
		m_diskTexInfo.shrink_to_fit();

		m_diskFaces.clear();
		m_diskFaces.shrink_to_fit();

		m_diskLeafFaces.clear();
		m_diskLeafFaces.shrink_to_fit();

		for (int i = 0; i < m_diskCompressedVis.numclusters; ++i) {
			delete[] m_diskCompressedVis.bitofs[i];  // Delete each row
		}
		delete[] m_diskCompressedVis.bitofs;  // Delete the array of pointers

		m_diskLeafs.clear();
		m_diskLeafs.shrink_to_fit();

		m_diskNodes.clear();
		m_diskNodes.shrink_to_fit();

		m_diskSubModels.clear();
		m_diskSubModels.shrink_to_fit();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::BSP38::Shutdown()
{
	try
	{
		ClearDiskData();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}