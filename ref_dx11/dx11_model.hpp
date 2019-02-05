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

#ifndef __DX11_MODEL_HPP__
#define __DX11_MODEL_HPP__
#pragma once

#include "dx11_local.hpp"

typedef struct dxVertex_s {
	XMFLOAT3 xyz;			// Position
	XMFLOAT3 normal;
	XMFLOAT4 tangent;
	XMFLOAT3 bitangent;
	XMFLOAT4 color;
	XMFLOAT2 textureCoordinate0;
	XMFLOAT2 textureCoordinate1;
} dxVertex;

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/


/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct mvertex_s
{
	vec3_t		position;
} mvertex_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct medge_s
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct mmodel_s
{
	vec3_t		mins, maxs;
	vec3_t		origin;		// for sounds or lights
	float		radius;
	int			headnode;
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} mmodel_t;

typedef struct mtexinfo_s
{
	float		vecs[2][4];
	int			flags;
	int			numframes;
	struct mtexinfo_s	*next;		// animation chain
	image_t		*image;
} mtexinfo_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	int			dlightframe;
	int			dlightbits;

	cplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

							// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t	*texinfo;

	// lighting info
	byte		styles[MAXLIGHTMAPS];
	byte		*samples;		// [numstyles*surfsize]

	struct msurface_s *nextalphasurface;
} msurface_t;

typedef struct mnode_s
{
	// common with leaf
	int			contents;		// -1, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

	// node specific
	cplane_t	*plane;
	struct mnode_s	*children[2];

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;

typedef struct mleaf_s
{
	// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

	// leaf specific
	int			cluster;
	int			area;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
} mleaf_t;

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
	byte		padding[3];
	vec3_t		clipmins, clipmaxs;

	//
	// brush model
	//
	int			firstmodelsurface, nummodelsurfaces;
	int			lightmap;		// only for submodels

	int			numsubmodels;
	mmodel_t	*submodels;

	int			numplanes;
	cplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	int			firstnode;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and skins
	image_t		*skins[MAX_MD2SKINS];

	int			extradatasize;
	void		*extradata;
} model_t;

namespace dx11
{
	class ModelManager 
	{
	public:
									ModelManager();

		bool						Initialize();
		void						Shutdown();

		std::shared_ptr<model_t>	Load(std::string modelName);
	};
}

/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
===============================================================================

	Render Model

===============================================================================
*/

// shared between the renderer, game, and Maya export DLL
#define MD5_VERSION_STRING		"MD5Version"
#define MD5_MESH_EXT			"md5mesh"
#define MD5_ANIM_EXT			"md5anim"
#define MD5_CAMERA_EXT			"md5camera"
#define MD5_VERSION				10

#ifdef ENABLE_D3_SHADOWS
#include "jobs/ShadowShared.h"
#include "jobs/prelightshadowvolume/PreLightShadowVolume.h"
#include "jobs/staticshadowvolume/StaticShadowVolume.h"
#include "jobs/dynamicshadowvolume/DynamicShadowVolume.h"
#endif // ENABLE_D3_SHADOWS

// this is used for calculating unsmoothed normals and tangents for deformed models
struct dominantTri_t {
	triIndex_t					v2, v3;
	float						normalizationScale[3];
};

#ifdef ENABLE_D3_SHADOWS
const int SHADOW_CAP_INFINITE = 64;
#endif // ENABLE_D3_SHADOWS

class idRenderModelStatic;
struct viewDef_t;

// our only drawing geometry type
struct srfTriangles_t {
	srfTriangles_t() {}

	BoundingOrientedBox			bounds;					// for culling

	bool						generateNormals;		// create normals from geometry, instead of using explicit ones
	bool						tangentsCalculated;		// set when the vertex tangents have been calculated
	bool						perfectHull;			// true if there aren't any dangling edges
	bool						referencedVerts;		// if true the 'verts' are referenced and should not be freed
	bool						referencedIndexes;		// if true, indexes, silIndexes, mirrorVerts, and silEdges are
														// pointers into the original surface, and should not be freed

	byte						padding[3];

	unsigned int				numVerts;				// number of vertices
	dxVertex *					verts;					// vertices, allocated with special allocator

	unsigned int				numIndexes;				// for shadows, this has both front and rear end caps and silhouette planes
	triIndex_t *				indexes;				// indexes, allocated with special allocator

#ifdef ENABLE_D3_SHADOWS
	triIndex_t *				silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords
#endif // ENABLE_D3_SHADOWS

	unsigned int				numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
	int *						mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->numVerts - tri->numMirroredVerts + 0

	unsigned int				numDupVerts;			// number of duplicate vertexes
	int *						dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex

#ifdef ENABLE_D3_SHADOWS
	int							numSilEdges;			// number of silhouette edges
	silEdge_t *					silEdges;				// silhouette edges
#endif // ENABLE_D3_SHADOWS

	dominantTri_t *				dominantTris;			// [numVerts] for deformed surface fast tangent calculation

#ifdef ENABLE_D3_SHADOWS
	int							numShadowIndexesNoFrontCaps;	// shadow volumes with front caps omitted
	int							numShadowIndexesNoCaps;			// shadow volumes with the front and rear caps omitted

	int							shadowCapPlaneBits;		// bits 0-5 are set when that plane of the interacting light has triangles
														// projected on it, which means that if the view is on the outside of that
														// plane, we need to draw the rear caps of the shadow volume
														// dynamic shadows will have SHADOW_CAP_INFINITE

	idShadowVert *				preLightShadowVertexes;	// shadow vertices in CPU memory for pre-light shadow volumes
	idShadowVert *				staticShadowVertexes;	// shadow vertices in CPU memory for static shadow volumes
#endif // ENABLE_D3_SHADOWS

	srfTriangles_t *			ambientSurface;			// for light interactions, point back at the original surface that generated
														// the interaction, which we will get the ambientCache from

	srfTriangles_t *			nextDeferredFree;		// chain of tris to free next frame

	byte						padding2[4];

	// for deferred normal / tangent transformations by joints
	// the jointsInverted list / buffer object on md5WithJoints may be
	// shared by multiple srfTriangles_t
	idRenderModelStatic *		staticModelWithJoints;

	byte						padding3[4];

	// data in vertex object space, not directly readable by the CPU
	uint64			indexCache;				// GL_INDEX_TYPE
	uint64			ambientCache;			// idDrawVert

#ifdef ENABLE_D3_SHADOWS
	uint64			shadowCache;			// idVec4
#endif // ENABLE_D3_SHADOWS

	DISALLOW_COPY_AND_ASSIGN(srfTriangles_t);
};

//typedef idList<srfTriangles_t *, TAG_IDLIB_LIST_TRIANGLES> idTriList;

struct modelSurface_t {
	int							id;
	const dx11::Texture2D *		texture;
	srfTriangles_t *			geometry;
};

enum dynamicModel_t {
	DM_STATIC,		// never creates a dynamic model
	DM_CACHED,		// once created, stays constant until the entity is updated (animating characters)
	DM_CONTINUOUS	// must be recreated for every single view (time dependent things like particles)
};

enum jointHandle_t {
	INVALID_JOINT = -1
};

class idMD5Joint {
public:
	idMD5Joint() { parent = NULL; }
	std::string					name;
	const idMD5Joint *			parent;
};


// the init methods may be called again on an already created model when
// a reloadModels is issued

class idRenderModel {
public:
	virtual						~idRenderModel() {};

	// Loads static models only, dynamic models must be loaded by the modelManager
	virtual void				InitFromFile(const char *fileName) = 0;

	// Supports reading/writing binary file formats
	virtual bool				LoadBinaryModel(std::string filename, const ID_TIME_T sourceTimeStamp) = 0;
	//virtual bool				LoadBinaryModel(idFile * file, const ID_TIME_T sourceTimeStamp) = 0;
	//virtual void				WriteBinaryModel(idFile * file, ID_TIME_T *_timeStamp = NULL) const = 0;
	virtual bool				SupportsBinaryModel() = 0;

	// renderBump uses this to load the very high poly count models, skipping the
	// shadow and tangent generation, along with some surface cleanup to make it load faster
	virtual void				PartialInitFromFile(const char *fileName) = 0;

	// this is used for dynamically created surfaces, which are assumed to not be reloadable.
	// It can be called again to clear out the surfaces of a dynamic model for regeneration.
	virtual void				InitEmpty(const char *name) = 0;

	// dynamic model instantiations will be created with this
	// the geometry data will be owned by the model, and freed when it is freed
	// the geoemtry should be raw triangles, with no extra processing
	virtual void				AddSurface(modelSurface_t surface) = 0;

	// cleans all the geometry and performs cross-surface processing
	// like shadow hulls
	// Creates the duplicated back side geometry for two sided, alpha tested, lit materials
	// This does not need to be called if none of the surfaces added with AddSurface require
	// light interaction, and all the triangles are already well formed.
	virtual void				FinishSurfaces() = 0;

	// frees all the data, but leaves the class around for dangling references,
	// which can regenerate the data with LoadModel()
	virtual void				PurgeModel() = 0;

	// resets any model information that needs to be reset on a same level load etc.. 
	// currently only implemented for liquids
	virtual void				Reset() = 0;

	// used for initial loads, reloadModel, and reloading the data of purged models
	// Upon exit, the model will absolutely be valid, but possibly as a default model
	virtual void				LoadModel() = 0;

	// internal use
	virtual bool				IsLoaded() = 0;
	virtual void				SetLevelLoadReferenced(bool referenced) = 0;
	virtual bool				IsLevelLoadReferenced() = 0;

	// models that are already loaded at level start time
	// will still touch their data to make sure they
	// are kept loaded
	virtual void				TouchData() = 0;

	// dump any ambient caches on the model surfaces
	virtual void				FreeVertexCache() = 0;

	// returns the name of the model
	virtual const char	*		Name() const = 0;

	// prints a detailed report on the model for printModel
	virtual void				Print() const = 0;

	// prints a single line report for listModels
	virtual void				List() const = 0;

	// reports the amount of memory (roughly) consumed by the model
	virtual int					Memory() const = 0;

	// for reloadModels
	virtual ID_TIME_T			Timestamp() const = 0;

	// returns the number of surfaces
	virtual int					NumSurfaces() const = 0;

	// NumBaseSurfaces will not count any overlays added to dynamic models
	virtual int					NumBaseSurfaces() const = 0;

	// get a pointer to a surface
	virtual const modelSurface_t *Surface(int surfaceNum) const = 0;

	// Allocates surface triangles.
	// Allocates memory for srfTriangles_t::verts and srfTriangles_t::indexes
	// The allocated memory is not initialized.
	// srfTriangles_t::numVerts and srfTriangles_t::numIndexes are set to zero.
	virtual srfTriangles_t *	AllocSurfaceTriangles(int numVerts, int numIndexes) const = 0;

	// Frees surfaces triangles.
	virtual void				FreeSurfaceTriangles(srfTriangles_t *tris) const = 0;

	// models of the form "_area*" may have a prelight shadow model associated with it
	virtual bool				IsStaticWorldModel() const = 0;

	// models parsed from inside map files or dynamically created cannot be reloaded by
	// reloadmodels
	virtual bool				IsReloadable() const = 0;

	// md3, md5, particles, etc
	virtual dynamicModel_t		IsDynamicModel() const = 0;

	// if the load failed for any reason, this will return true
	virtual bool				IsDefaultModel() const = 0;

	// dynamic models should return a fast, conservative approximation
	// static models should usually return the exact value
	virtual BoundingOrientedBox		Bounds(const struct renderEntity_s *ent = NULL) const = 0;

	// returns value != 0.0f if the model requires the depth hack
	virtual float				DepthHack() const = 0;

	// returns a static model based on the definition and view
	// currently, this will be regenerated for every view, even though
	// some models, like character meshes, could be used for multiple (mirror)
	// views in a frame, or may stay static for multiple frames (corpses)
	// The renderer will delete the returned dynamic model the next view
	// This isn't const, because it may need to reload a purged model if it
	// wasn't precached correctly.
	virtual idRenderModel *		InstantiateDynamicModel(const struct renderEntity_s *ent, const viewDef_t *view, idRenderModel *cachedModel) = 0;

	// Returns the number of joints or 0 if the model is not an MD5
	virtual int					NumJoints() const = 0;

	// Returns the MD5 joints or NULL if the model is not an MD5
	virtual const idMD5Joint *	GetJoints() const = 0;

	// Returns the handle for the joint with the given name.
	virtual jointHandle_t		GetJointHandle(const char *name) const = 0;

	// Returns the name for the joint with the given handle.
	virtual const char *		GetJointName(jointHandle_t handle) const = 0;

	// Returns the default animation pose or NULL if the model is not an MD5.
	//virtual const idJointQuat *	GetDefaultPose() const = 0;

	// Returns number of the joint nearest to the given triangle.
	virtual int					NearestJoint(int surfaceNum, int a, int c, int b) const = 0;

	// Writing to and reading from a demo file.
	virtual void				ReadFromDemoFile(class idDemoFile *f) = 0;
	virtual void				WriteToDemoFile(class idDemoFile *f) = 0;

	// if false, the model doesn't need to be linked into the world, because it
	// can't contribute visually -- triggers, etc
	virtual bool				ModelHasDrawingSurfaces() const { return true; };

	// if false, the model doesn't generate interactions with lights
	virtual bool				ModelHasInteractingSurfaces() const { return true; };

	// if false, the model doesn't need to be added to the view unless it is
	// directly visible, because it can't cast shadows into the view
	virtual bool				ModelHasShadowCastingSurfaces() const { return true; };
};

#endif // !__DX11_MODEL_HPP__