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

#include "dx12_local.hpp"

dx12::Cvars::Cvars()
{
	forceScreenRes		= std::make_shared<Cvar>("dx12_forceScreenRes",		1,								CVAR_ARCHIVE);
	featureLevel		= std::make_shared<Cvar>("dx12_featureLevel",		"D3D_FEATURE_LEVEL_12_1",		CVAR_ARCHIVE);
	bufferCount			= std::make_shared<Cvar>("dx12_bufferCount",		2,								CVAR_ARCHIVE);
	backBufferFormat	= std::make_shared<Cvar>("dx12_backBufferFormat",	"DXGI_FORMAT_R8G8B8A8_UNORM",	CVAR_ARCHIVE);
	Vsync				= std::make_shared<Cvar>("dx12_Vsync",				1,								CVAR_ARCHIVE);
	samplesPerPixel		= std::make_shared<Cvar>("dx12_samplesPerPixel",	2,								CVAR_ARCHIVE);	// 1 disables multisampling
}