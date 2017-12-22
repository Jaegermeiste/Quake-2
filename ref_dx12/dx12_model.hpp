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
2017 Bleeding Eye Studios
*/

#ifndef __DX12_MODEL_HPP__
#define __DX12_MODEL_HPP__
#pragma once

#include "dx12_local.hpp"

//
// Whole model
//

typedef enum { mod_bad, mod_brush, mod_sprite, mod_alias } modtype_t;

typedef struct model_s
{
	char		name[MAX_QPATH];

	int			registration_sequence;

	modtype_t	type;
	int			numframes;

	int			flags;

	//
	// volume occupied by the model graphics
	//		
	vec3_t		mins, maxs;
	float		radius;

	//
	// solid volume for clipping 
	//
	qboolean	clipbox;
	vec3_t		clipmins, clipmaxs;

	//
	// brush model
	//
	int			firstmodelsurface, nummodelsurfaces;
	int			lightmap;		// only for submodels

	int			numsubmodels;
	//mmodel_t	*submodels;

	int			numplanes;
	cplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	//mleaf_t		*leafs;

	int			numvertexes;
	//mvertex_t	*vertexes;

	int			numedges;
	//medge_t		*edges;

	int			numnodes;
	int			firstnode;
	//mnode_t		*nodes;

	int			numtexinfo;
	//mtexinfo_t	*texinfo;

	int			numsurfaces;
	//msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	//msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and skins
	image_t		*skins[MAX_MD2SKINS];

	int			extradatasize;
	void		*extradata;
} model_t;

namespace dx12
{
	class Model {
	private:

	public:
		std::shared_ptr<model_t>	LoadMap(std::string name);
		std::shared_ptr<model_t>	LoadModel(std::string name);
	};
}

#endif // !__DX12_MODEL_HPP__