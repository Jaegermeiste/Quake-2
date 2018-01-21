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

#include "dx11_local.hpp"

// Global variable (per MS documentation)
CRITICAL_SECTION CriticalSection;

#ifdef _DEBUG
ID3D11Debug* d3dDebug = nullptr;
ID3D11InfoQueue *d3dInfoQueue = nullptr;
#endif

/*
===============
dx11::Initialize
===============
*/
bool dx11::Initialize()
{
	LOG_FUNC();

	if (dx11::ref->client != nullptr)
	{
		dx11::ref->client->Con_Printf(PRINT_ALL, "ref_dx11 version: " REF_VERSION);
	}

	// Initialize the critical section one time only.
	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
	{
		return false;
	}


	return true;
}

/*
===============
dx11::Shutdown
===============
*/
void dx11::Shutdown()
{
	LOG_FUNC();

	// Release resources used by the critical section object.
	DeleteCriticalSection(&CriticalSection);

#ifdef _DEBUG
	if (d3dDebug)
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);

		SAFE_RELEASE(d3dDebug);
	}
#endif

}