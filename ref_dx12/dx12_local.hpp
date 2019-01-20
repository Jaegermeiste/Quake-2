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

#ifndef __DX12_LOCAL_HPP__
#define __DX12_LOCAL_HPP__
#pragma once

#include "stdafx.h"

#include "../client/ref.h"
#include "../win32/winquake.h"

#pragma warning(default:4100)	// Unreferenced formal parameter
#pragma warning(default:4242)	// Possible loss of data
#pragma warning(default:4365)	// Signed/unsigned mismatch
#pragma warning(default:4820)	// Padding

extern	unsigned short	BigUShort(unsigned short l);
extern	unsigned short	LittleUShort(unsigned short l);
extern	unsigned int	BigULong(unsigned int l);
extern	unsigned int	LittleULong(unsigned int l);

#define	REF_VERSION	"DX12 " __DATE__ " " __TIME__

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

// Courtesy https://stackoverflow.com/questions/195975/how-to-make-a-char-string-from-a-c-macros-value
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

// Handles
typedef int			qhandle_t;
typedef std::size_t handle_t;

typedef enum rserr_e
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

typedef struct viddef_s
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

typedef enum imagetype_e
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

typedef struct image_s
{
	char				name[MAX_QPATH];				// game path, including extension
	imagetype_t			type;
	int					width, height;					// source image
	int					upload_width, upload_height;	// after power of two and picmip
	int					registration_sequence;			// 0 = free
	struct msurface_s	*texturechain;					// for sort-by-texture world drawing
	int					texnum;							// gl texture binding
	float				sl, tl, sh, th;					// 0,0 - 1,1 unless part of the scrap
	qboolean			scrap;
	qboolean			has_alpha;

	qboolean			paletted;

	byte				padding[1];
} image_t;

#define EMA_ALPHA	0.9

#define SAFE_RELEASE(comObject)	if (comObject) { comObject->Release(); comObject = nullptr; }

extern CRITICAL_SECTION CriticalSection;
#ifdef _DEBUG
extern ID3D12Debug* d3dDebug;
extern ID3D12DebugDevice* d3dDebugDev;
extern ID3D12InfoQueue *d3dInfoQueue;
#endif

namespace dx12 {
	typedef __declspec(align(16)) struct Vertex2D_s
	{
		DirectX::XMFLOAT4A		position;
		DirectX::XMVECTORF32	color;
		DirectX::XMFLOAT2A		texCoord;
	} Vertex2D;
}

#include "dx12_log.hpp"
#include "dx12_cvar.hpp"
#include "dx12_resource.hpp"
#include "dx12_shader.hpp"
#include "dx12_web.hpp"
#include "dx12_draw.hpp"
#include "dx12_quad2D.hpp"
#include "dx12_subsystem2D.hpp"
#include "dx12_subsystem3D.hpp"
#include "dx12_subsystemText.hpp"
#include "dx12_dx.hpp"
#include "dx12_system.hpp"
#include "dx12_client.hpp"
#include "dx12_texture2D.hpp"
#include "dx12_image.hpp"
#include "dx12_model.hpp"
#include "dx12_light.hpp"
#include "dx12_xplit.hpp"
#include "dx12_bsp.hpp"
#include "dx12_map.hpp"
#include "dx12_media.hpp"
#include "dx12_ref.hpp"

namespace dx12
{
	// Functions
	bool			Initialize();
	void			Shutdown();

	void			DumpD3DDebugMessagesToLog();
}

#endif // !__DX12_LOCAL_HPP__
