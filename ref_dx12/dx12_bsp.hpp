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
2025 Bleeding Eye Studios
*/

#ifndef __DX12_BSP_HPP__
#define __DX12_BSP_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	typedef struct disk_bsp_header_s
	{
		int			ident;
		int			version;
		lump_t		*lumps;
	} bsp_disk_header;

	class BSP
	{
	protected:
		std::wstring				m_name;
		unsigned int				m_version;

		size_t				        m_numVertices;
		Vertex3D*					m_vertices;

		std::vector<int>            m_indices;

		bsp_disk_header*			m_header = nullptr;

		size_t		                LoadDiskVertices_v29_v38(void* data, unsigned int offset, size_t length);
		virtual size_t      		LoadVertices() = 0;

	public:
		virtual size_t      	    LoadLighting() { return 0; };
		virtual void                Upload(std::shared_ptr<CommandList> commandList) = 0;
	};

constexpr auto  BSP38_VERSION          = 38;
constexpr auto	BSP38_LUMP_ENTITIES    = 0;
constexpr auto  BSP38_LUMP_PLANES      = 1;
constexpr auto  BSP38_LUMP_VERTEXES    = 2;
constexpr auto	BSP38_LUMP_VISIBILITY  = 3;
constexpr auto	BSP38_LUMP_NODES		   = 4;
constexpr auto	BSP38_LUMP_TEXINFO	   = 5;
constexpr auto	BSP38_LUMP_FACES		   = 6;
constexpr auto	BSP38_LUMP_LIGHTING	   = 7;
constexpr auto	BSP38_LUMP_LEAFS		   = 8;
constexpr auto	BSP38_LUMP_LEAFFACES	   = 9;
constexpr auto	BSP38_LUMP_LEAFBRUSHES = 10;
constexpr auto	BSP38_LUMP_EDGES		   = 11;
constexpr auto	BSP38_LUMP_SURFEDGES	   = 12;
constexpr auto	BSP38_LUMP_MODELS	   = 13;
constexpr auto	BSP38_LUMP_BRUSHES	   = 14;
constexpr auto	BSP38_LUMP_BRUSHSIDES  = 15;
constexpr auto	BSP38_LUMP_POP		   = 16;
constexpr auto	BSP38_LUMP_AREAS		   = 17;
constexpr auto	BSP38_LUMP_AREAPORTALS = 18;
constexpr auto	BSP38_HEADER_LUMPS	   = 19;

typedef struct disk_bsp38_header_s
{
	int			ident;
	int			version;
	lump_t      lumps[BSP38_HEADER_LUMPS];
} bsp38_disk_header;

typedef struct disk_bsp38_plane_s
{
	float	normal[3];
	float	dist;
	int		type;
} bsp38_plane_t;

typedef struct disk_bsp38_edge_s
{
	unsigned short	index[2];
	unsigned int	cachedEdgeOffset;
} bsp38_edge_t;

typedef struct disk_bsp38_vis_s
{
	int			numclusters;
	int** bitofs;// [] [2] ;	// bitofs[numclusters][2]
} bsp38_vis_t;

typedef struct bsp38_leaf_geometry_s
{
	size_t                     index = 0;
	int			               cluster = 0;
	int			               area = 0;
	std::vector<Vertex3D>      vertices;     // A collection of unique vertices in the leaf
	std::vector<unsigned long> indices;  // Indices forming triangles from the vertex list
} bsp38_leaf_geometry_t;

	class BSP38 : public BSP
	{
	private:
		// DISK
		bsp38_disk_header*                 m_header38 = nullptr;

		std::vector<DirectX::XMFLOAT3A>	   m_diskVertices;

		std::vector<dedge_t>               m_diskEdges;

		std::vector<int>                   m_diskSurfEdges;

		std::vector<dx12::Light>           m_lights;

		std::vector<bsp38_plane_t>         m_diskPlanes;

		std::vector<texinfo_t>		       m_diskTexInfo;

		std::vector<dface_t>               m_diskFaces;

		std::vector<short>                 m_diskLeafFaces;

		bsp38_vis_t                        m_diskCompressedVis = {};

		std::vector<dleaf_t>               m_diskLeafs;

		std::vector<dnode_t>               m_diskNodes;

		std::vector<dmodel_t>              m_diskSubModels;

		// COMPUTE
		std::vector<bsp38_leaf_geometry_t> m_leafGeometry;

		std::vector<BottomLevelAccelerationStructure> m_leafAccelStructures;

		// Functions
		size_t                             LoadVertices() override;
		size_t                             LoadEdges();
		size_t                             LoadSurfEdges();
		size_t                             LoadLighting();
		size_t                             LoadPlanes();
		size_t                             LoadTexInfo();
		size_t                             LoadFaces();
		size_t                             LoadLeafFaces();
		size_t                             LoadVisibility();
		size_t                             LoadLeafs();
		size_t                             LoadNodes();
		size_t                             LoadSubModels();

		void                               CollectLeafGeometry();
		void                               UploadLeafGeometry();
		void                               GenerateBottomLevelAS(std::shared_ptr<CommandList> commandList);
		void                               GenerateTopLevelAS(std::shared_ptr<CommandList> commandList);
		
	public:
							               BSP38(std::wstring name, unsigned int* buffer);
							               ~BSP38();

	    void                               ClearDiskData();

		void                               Shutdown();

		void                               ComputeIndices();

		void                               Upload(std::shared_ptr<CommandList> commandList) override;
	};

constexpr auto BSP46_VERSION = 46;
constexpr auto BSP47_VERSION = 47;
}

#endif // !__DX12_BSP_HPP__