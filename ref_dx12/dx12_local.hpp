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

#define	REF_VERSION	"DX12 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

#include "dx12_client.hpp"
#include "dx12_system.hpp"
#include "dx12_draw.hpp"

namespace dx12
{
	typedef struct
	{
		unsigned		width, height;			// coordinates from main game
	} viddef_t;

	typedef enum
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
	} image_t;

	// Functions
	qboolean	dx12::Init		(void *hInstance, void *wndProc);
	void		dx12::Shutdown	(void);
}

#endif // !__DX12_LOCAL_HPP__
