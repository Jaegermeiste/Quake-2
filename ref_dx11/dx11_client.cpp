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

inline void dx11::Client::Sys_Error(unsigned short err_level, std::string str)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Sys_Error(err_level, const_cast<char*>(str.c_str()));
}

inline void dx11::Client::Cmd_AddCommand(std::string name, void(*cmd)(void))
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Cmd_AddCommand(const_cast<char*>(name.c_str()), cmd);
}

inline void dx11::Client::Cmd_RemoveCommand(std::string name)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Cmd_RemoveCommand(const_cast<char*>(name.c_str()));
}

inline unsigned int dx11::Client::Cmd_Argc (void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return msl::utilities::SafeInt<unsigned int>(ri.Cmd_Argc());
}

inline std::string dx11::Client::Cmd_Argv(unsigned int i)
{
	int clientIndex = msl::utilities::SafeInt<int>(i);

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return ri.Cmd_Argv(clientIndex);
}

inline void dx11::Client::Cmd_ExecuteText		(unsigned int exec_when, std::string text)
{
	int clientWhen = msl::utilities::SafeInt<int>(exec_when);

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Cmd_ExecuteText(clientWhen, const_cast<char*>(text.c_str()));
}

inline void dx11::Client::Con_Printf(unsigned short print_level, std::string str)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Con_Printf(print_level, const_cast<char*>(str.c_str()));
}

inline int dx11::Client::FS_LoadFile (std::string fileName, void **buf)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return ri.FS_LoadFile(const_cast<char*>(fileName.c_str()), buf);
}

inline void dx11::Client::FS_FreeFile(void *buf)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.FS_FreeFile(buf);
}

inline std::string dx11::Client::FS_Gamedir(void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	return std::string(ri.FS_Gamedir());
}

//cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
//cvar_t			*(*Cvar_Set)			(char *name, char *value);
//void(*Cvar_SetValue)		(char *name, float value);

inline bool dx11::Client::Vid_GetModeInfo(unsigned int &width, unsigned int &height, int mode)
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

inline void dx11::Client::Vid_MenuInit(void)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Vid_MenuInit();
}

inline void dx11::Client::Vid_NewWindow(unsigned int width, unsigned int height)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(refImportMutex);

	ri.Vid_NewWindow(msl::utilities::SafeInt<int>(width), msl::utilities::SafeInt<int>(height));
}

void dx11::Client::SetRefImport(refimport_t rimp)
{ 
	ri = rimp;

	// Pass everything through that isn't explicitly overridden in the class functions above
	dx11::Client::Cvar_Get			= ri.Cvar_Get;
	dx11::Client::Cvar_Set			= ri.Cvar_Set;
	dx11::Client::Cvar_SetValue		= ri.Cvar_SetValue;
};

dx11::Client::Client(refimport_t rimp)
{
	SetRefImport(rimp);
}

dx11::Client::~Client()
{
	// Null everything through that isn't explicitly overridden in the class functions above
	dx11::Client::Cvar_Get			= nullptr;
	dx11::Client::Cvar_Set			= nullptr;
	dx11::Client::Cvar_SetValue		= nullptr;
}