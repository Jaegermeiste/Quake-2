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

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t	re;

	if (dx12::client == nullptr)
	{
		dx12::client = new dx12::Client(rimp);
	}
	else
	{
		dx12::client->SetRefImport(rimp);
	}

	re.api_version			= API_VERSION;

	re.BeginRegistration	= R_BeginRegistration;
	re.RegisterModel		= R_RegisterModel;
	re.RegisterSkin			= R_RegisterSkin;
	re.RegisterPic			= dx12::draw->FindPic;
	re.SetSky				= R_SetSky;
	re.EndRegistration		= R_EndRegistration;

	re.RenderFrame			= R_RenderFrame;

	re.DrawGetPicSize		= dx12::draw->GetPicSize;
	re.DrawPic				= dx12::draw->Pic;
	re.DrawStretchPic		= dx12::draw->StretchPic;
	re.DrawChar				= dx12::draw->Char;
	re.DrawTileClear		= dx12::draw->TileClear;
	re.DrawFill				= dx12::draw->Fill;
	re.DrawFadeScreen		= dx12::draw->FadeScreen;
	re.DrawStretchRaw		= dx12::draw->StretchRaw;

	re.Init					= dx12::Init;
	re.Shutdown				= dx12::Shutdown;

	re.CinematicSetPalette	= R_SetPalette;
	re.BeginFrame			= R_BeginFrame;
	re.EndFrame				= GLimp_EndFrame;

	re.AppActivate			= GLimp_AppActivate;

	Swap_Init();

	return re;
}