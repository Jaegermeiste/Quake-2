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

/*
===============
dx11::Init
===============
*/
bool dx11::Init(HINSTANCE hInstance, WNDPROC wndProc)
{
	BOOST_LOG_NAMED_SCOPE("Init");

	if (dx11::ref->client != nullptr)
	{
		ref->client->Con_Printf(PRINT_ALL, "ref_dx11 version: " REF_VERSION "\n");
	}

	return true;
}

/*
===============
dx11::Shutdown
===============
*/
void dx11::Shutdown(void)
{
	BOOST_LOG_NAMED_SCOPE("Shutdown");
	// Clean up
	// ref is a smart pointer - it will call the destructor when dereferenced
}