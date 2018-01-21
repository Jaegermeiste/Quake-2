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

#ifndef __DX11_LOCAL_HPP__
#define __DX11_LOCAL_HPP__
#pragma once

#include "stdafx.h"
#include <DirectXMath.h>

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

#define	REF_VERSION	"DX11 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

// Courtesy https://stackoverflow.com/questions/195975/how-to-make-a-char-string-from-a-c-macros-value
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

typedef struct viddef_s
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

typedef enum rserr_e
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

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
extern ID3D11Debug* d3dDebug;
extern ID3D11InfoQueue *d3dInfoQueue;
#endif

#include "dx11_log.hpp"
#include "dx11_cvar.hpp"
#include "dx11_shader.hpp"
#include "dx11_web.hpp"
#include "dx11_subsystem2D.hpp"
#include "dx11_subsystem3D.hpp"
#include "dx11_subsystemText.hpp"
#include "dx11_dx.hpp"
#include "dx11_system.hpp"
#include "dx11_client.hpp"
#include "dx11_image.hpp"
#include "dx11_model.hpp"
#include "dx11_draw.hpp"
#include "dx11_ref.hpp"

namespace dx11
{
	// Functions
	bool			Initialize	();
	void			Shutdown	();
}

#endif // !__DX11_LOCAL_HPP__
