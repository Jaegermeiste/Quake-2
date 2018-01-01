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

inline void dx12::Client::Sys_Error(unsigned short err_level, std::string str)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Sys_Error(err_level, const_cast<char*>(str.c_str()));
}

// void(*Cmd_AddCommand)		(char *name, void(*cmd)(void));

inline void dx12::Client::Cmd_RemoveCommand(std::string name)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Cmd_RemoveCommand(const_cast<char*>(name.c_str()));
}

inline unsigned int dx12::Client::Cmd_Argc (void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return msl::utilities::SafeInt<unsigned int>(ri.Cmd_Argc());
}

//char			*(*Cmd_Argv)			(int i);
//void(*Cmd_ExecuteText)		(int exec_when, char *text);

inline void dx12::Client::Con_Printf(unsigned short print_level, std::string str)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Con_Printf(print_level, const_cast<char*>(str.c_str()));
}

inline int dx12::Client::FS_LoadFile (std::string fileName, void **buf)
{
	return ri.FS_LoadFile(const_cast<char*>(fileName.c_str()), buf);
}

//int(*FS_LoadFile)			(char *name, void **buf);
//void(*FS_FreeFile)			(void *buf);

inline std::string dx12::Client::FS_Gamedir(void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return std::string(ri.FS_Gamedir());
}

//cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
//cvar_t			*(*Cvar_Set)			(char *name, char *value);
//void(*Cvar_SetValue)		(char *name, float value);

inline bool dx12::Client::Vid_GetModeInfo(unsigned int &width, unsigned int &height, int mode)
{
	int clientWidth = 0,
		clientHeight = 0;

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	qboolean retVal = ri.Vid_GetModeInfo(&clientWidth, &clientHeight, mode);

	if (retVal = qtrue)
	{
		width = msl::utilities::SafeInt<int>(clientWidth);
		height = msl::utilities::SafeInt<int>(clientHeight);
		return true;
	}
	return false;
}

inline void dx12::Client::Vid_MenuInit(void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Vid_MenuInit();
}

inline void dx12::Client::Vid_NewWindow(unsigned int width, unsigned int height)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Vid_NewWindow(msl::utilities::SafeInt<int>(width), msl::utilities::SafeInt<int>(height));
}

void dx12::Client::SetRefImport(refimport_t rimp)
{ 
	ri = rimp;

	// Pass everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cmd_AddCommand	= ri.Cmd_AddCommand;
	dx12::Client::Cmd_Argv			= ri.Cmd_Argv;
	dx12::Client::Cmd_ExecuteText	= ri.Cmd_ExecuteText;
	dx12::Client::Cvar_Get			= ri.Cvar_Get;
	dx12::Client::Cvar_Set			= ri.Cvar_Set;
	dx12::Client::Cvar_SetValue		= ri.Cvar_SetValue;
	dx12::Client::FS_FreeFile		= ri.FS_FreeFile;
};

dx12::Client::Client(refimport_t rimp)
{
	SetRefImport(rimp);
}

dx12::Client::~Client()
{
	// Null everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cmd_AddCommand	= nullptr;
	dx12::Client::Cmd_Argv			= nullptr;
	dx12::Client::Cmd_ExecuteText	= nullptr;
	dx12::Client::Cvar_Get			= nullptr;
	dx12::Client::Cvar_Set			= nullptr;
	dx12::Client::Cvar_SetValue		= nullptr;
	dx12::Client::FS_FreeFile		= nullptr;
}