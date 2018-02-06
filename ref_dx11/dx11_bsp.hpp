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

#ifndef __DX11_BSP_HPP__
#define __DX11_BSP_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class BSP
	{
	protected:
		std::string		m_name;
		unsigned int	m_version;
	};

#define BSP38_VERSION			38
#define	BSP38_LUMP_ENTITIES		0
#define	BSP38_LUMP_PLANES		1
#define	BSP38_LUMP_VERTEXES		2
#define	BSP38_LUMP_VISIBILITY	3
#define	BSP38_LUMP_NODES		4
#define	BSP38_LUMP_TEXINFO		5
#define	BSP38_LUMP_FACES		6
#define	BSP38_LUMP_LIGHTING		7
#define	BSP38_LUMP_LEAFS		8
#define	BSP38_LUMP_LEAFFACES	9
#define	BSP38_LUMP_LEAFBRUSHES	10
#define	BSP38_LUMP_EDGES		11
#define	BSP38_LUMP_SURFEDGES	12
#define	BSP38_LUMP_MODELS		13
#define	BSP38_LUMP_BRUSHES		14
#define	BSP38_LUMP_BRUSHSIDES	15
#define	BSP38_LUMP_POP			16
#define	BSP38_LUMP_AREAS		17
#define	BSP38_LUMP_AREAPORTALS	18
#define	BSP38_HEADER_LUMPS		19

	typedef struct disk_bsp_header_s
	{
		int			ident;
		int			version;
		lump_t		*lumps;
	} disk_bsp_header_t;

	class BSP38 : public BSP
	{
	private:
		static bool			DownloadXPLitForMap(std::string mapName);

		void				LoadLighting();

	protected:
		//
		// brush model
		//
		unsigned int		m_firstModelSurface, 
							m_numModelSurfaces;
		unsigned int		m_lightmap;		// only for submodels

		unsigned int		m_numSubModels;
		mmodel_t*			m_subModels;

		unsigned int		m_numPlanes;
		cplane_t*			m_planes;

		unsigned int		m_numLeafs;		// number of visible leafs, not counting 0
		mleaf_t*			m_leafs;

		unsigned int		m_numVertices;
		DirectX::XMFLOAT3A*	m_vertices;

		unsigned int		m_numEdges;
		medge_t*			m_edges;

		unsigned int		m_numNodes;
		unsigned int		m_firstNode;
		mnode_t*			m_nodes;

		unsigned int		m_numTexInfo;
		mtexinfo_t*			m_texInfo;

		unsigned int		m_numSurfaces;
		msurface_t*			m_surfaces;

		unsigned int		m_numSurfEdges;
		int*				m_surfEdges;

		unsigned int		m_numMarkSurfaces;
		msurface_t**		m_markSurfaces;

		dvis_t*				m_vis;

		byte*				m_lightdata;
	public:
							BSP38(std::string name, unsigned int* buffer);
							~BSP38();
	};

#define BSP46_VERSION	46
#define BSP47_VERSION	47
}

#endif // !__DX11_BSP_HPP__