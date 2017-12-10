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

void dx12::Client::Sys_Error(unsigned short err_level, std::string str)
{
	ri.Sys_Error(err_level, const_cast<char*>(str.c_str()));
}

void dx12::Client::Con_Printf(unsigned short print_level, std::string str)
{
	ri.Con_Printf(print_level, const_cast<char*>(str.c_str()));
}

void dx12::Client::SetRefImport(refimport_t rimp) 
{ 
	ri = rimp;

	// Pass everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cmd_AddCommand	= ri.Cmd_AddCommand;
	dx12::Client::Cmd_Argc			= ri.Cmd_Argc;
	dx12::Client::Cmd_Argv			= ri.Cmd_Argv;
	dx12::Client::Cmd_ExecuteText	= ri.Cmd_ExecuteText;
	dx12::Client::Cmd_RemoveCommand	= ri.Cmd_RemoveCommand;
	dx12::Client::Cvar_Get			= ri.Cvar_Get;
	dx12::Client::Cvar_Set			= ri.Cvar_Set;
	dx12::Client::Cvar_SetValue		= ri.Cvar_SetValue;
	dx12::Client::FS_FreeFile		= ri.FS_FreeFile;
	dx12::Client::FS_Gamedir		= ri.FS_Gamedir;
	dx12::Client::FS_LoadFile		= ri.FS_LoadFile;
	dx12::Client::Vid_GetModeInfo	= ri.Vid_GetModeInfo;
	dx12::Client::Vid_MenuInit		= ri.Vid_MenuInit;
	dx12::Client::Vid_NewWindow		= ri.Vid_NewWindow;
};

dx12::Client::Client(refimport_t rimp)
{
	SetRefImport(rimp);
}

dx12::Client::~Client()
{
	// Null everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cmd_AddCommand	= nullptr;
	dx12::Client::Cmd_Argc			= nullptr;
	dx12::Client::Cmd_Argv			= nullptr;
	dx12::Client::Cmd_ExecuteText	= nullptr;
	dx12::Client::Cmd_RemoveCommand	= nullptr;
	dx12::Client::Cvar_Get			= nullptr;
	dx12::Client::Cvar_Set			= nullptr;
	dx12::Client::Cvar_SetValue		= nullptr;
	dx12::Client::FS_FreeFile		= nullptr;
	dx12::Client::FS_Gamedir		= nullptr;
	dx12::Client::FS_LoadFile		= nullptr;
	dx12::Client::Vid_GetModeInfo	= nullptr;
	dx12::Client::Vid_MenuInit		= nullptr;
	dx12::Client::Vid_NewWindow		= nullptr;
}