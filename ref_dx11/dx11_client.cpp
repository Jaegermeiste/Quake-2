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

void dx11::Client::Sys_Error(unsigned short err_level, std::string str)
{
	LOG_FUNC();

	std::string errLevelStr = "ERR_FATAL";

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	if (err_level == ERR_DROP)
	{
		errLevelStr = "ERR_DROP";
		LOG(error) << "<err_level> " << errLevelStr << " <string> " << str;
	}
	else if (err_level == ERR_QUIT)
	{
		errLevelStr = "ERR_QUIT";
		LOG(error) << "<err_level> " << errLevelStr << " <string> " << str;
	}
	else
	{
		LOG(fatal) << "<err_level> " << errLevelStr << " <string> " << str;
	}

	m_refImport.Sys_Error(err_level, const_cast<char*>(str.c_str()));
}

void dx11::Client::Cmd_AddCommand(std::string name, void(*cmd)())
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<name> " << name << " <cmd> " << cmd;

	m_refImport.Cmd_AddCommand(const_cast<char*>(name.c_str()), cmd);
}

void dx11::Client::Cmd_RemoveCommand(std::string name)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<name> " << name;

	m_refImport.Cmd_RemoveCommand(const_cast<char*>(name.c_str()));
}

unsigned int dx11::Client::Cmd_Argc (void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return msl::utilities::SafeInt<unsigned int>(m_refImport.Cmd_Argc());
}

std::string dx11::Client::Cmd_Argv(unsigned int i)
{
	LOG_FUNC();

	int clientIndex = msl::utilities::SafeInt<int>(i);

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return m_refImport.Cmd_Argv(clientIndex);
}

void dx11::Client::Cmd_ExecuteText		(unsigned int exec_when, std::string text)
{
	LOG_FUNC();

	int clientWhen = msl::utilities::SafeInt<int>(exec_when);

	std::string execWhenStr = "EXEC_NOW";

	if (exec_when == EXEC_INSERT)
	{
		execWhenStr = "EXEC_INSERT";
	}
	else if (exec_when == EXEC_APPEND)
	{
		execWhenStr = "EXEC_APPEND";
	}

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<exec_when> " << execWhenStr << " <text> " << text;

	m_refImport.Cmd_ExecuteText(clientWhen, const_cast<char*>(text.c_str()));
}

void dx11::Client::Con_Printf(unsigned short print_level, std::string str)
{
	LOG_FUNC();

	std::string printLevelStr = "PRINT_ALL";

	if (print_level == PRINT_DEVELOPER)
	{
		printLevelStr = "PRINT_DEVELOPER";
	}

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<print_level> " << printLevelStr << " <string> " << str;

	str += "\n";

	m_refImport.Con_Printf(print_level, const_cast<char*>(str.c_str()));
}

int dx11::Client::FS_LoadFile (std::string fileName, void **buf)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<fileName> " << fileName << " <buf> " << buf;

	return m_refImport.FS_LoadFile(const_cast<char*>(fileName.c_str()), buf);
}

void dx11::Client::FS_FreeFile(void *buf)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<buf> " << buf;

	m_refImport.FS_FreeFile(buf);
}

std::string dx11::Client::FS_Gamedir(void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return std::string(m_refImport.FS_Gamedir());
}

std::string dx11::Client::FS_GamedirAbsolute(void)
{
	LOG_FUNC();

	TCHAR  absoluteGamedirBuffer[4096] = TEXT("");
	ZeroMemory(&absoluteGamedirBuffer, sizeof(TCHAR) * 4096);

	std::string gameDir = FS_Gamedir();

	// Request ownership of the critical section.
	LOG(trace) << "Entering critical section";
	EnterCriticalSection(&CriticalSection);
	LOG(trace) << "Entered critical section";

	DWORD  retval = GetFullPathName(gameDir.c_str(), MAX_PATH, absoluteGamedirBuffer, NULL);

	// Release ownership of the critical section.
	LOG(trace) << "Leaving critical section";
	LeaveCriticalSection(&CriticalSection);
	LOG(trace) << "Left critical section";

	if (retval == 0)
	{
		LOG(error) << "GetFullPathName failed: " << GetLastError();
		return std::string();
	}
	else
	{
		LOG(info) << "The full path name is: " << absoluteGamedirBuffer;
	}

	return absoluteGamedirBuffer;
}

//cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
//cvar_t			*(*Cvar_Set)			(char *name, char *value);
//void(*Cvar_SetValue)		(char *name, float value);

bool dx11::Client::Vid_GetModeInfo(unsigned int &width, unsigned int &height, int mode)
{
	LOG_FUNC();

	int clientWidth = 0,
		clientHeight = 0;

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	qboolean retVal = m_refImport.Vid_GetModeInfo(&clientWidth, &clientHeight, mode);

	LOG(info) << "<width> " << width << " <height> " << height << " <mode> " << mode << " {return value} " << retVal;

	if (retVal = true)
	{
		width = msl::utilities::SafeInt<int>(clientWidth);
		height = msl::utilities::SafeInt<int>(clientHeight);
		return true;
	}
	return false;
}

void dx11::Client::Vid_MenuInit(void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	m_refImport.Vid_MenuInit();
}

void dx11::Client::Vid_NewWindow(unsigned int width, unsigned int height)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	m_refImport.Vid_NewWindow(msl::utilities::SafeInt<int>(width), msl::utilities::SafeInt<int>(height));
}

void dx11::Client::SetRefImport(refimport_t rimp)
{ 
	LOG_FUNC();

	m_refImport = rimp;

	// Pass everything through that isn't explicitly overridden in the class functions above
	dx11::Client::Cvar_Get			= m_refImport.Cvar_Get;
	dx11::Client::Cvar_Set			= m_refImport.Cvar_Set;
	dx11::Client::Cvar_SetValue		= m_refImport.Cvar_SetValue;
};

dx11::Client::Client(refimport_t rimp)
{
	LOG_FUNC();

	SetRefImport(rimp);
}

dx11::Client::~Client()
{
	// Null everything through that isn't explicitly overridden in the class functions above
	dx11::Client::Cvar_Get			= nullptr;
	dx11::Client::Cvar_Set			= nullptr;
	dx11::Client::Cvar_SetValue		= nullptr;
}

#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error(char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	dx11::ref->client->Sys_Error(ERR_FATAL, text);
}

void Com_Printf(char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	dx11::ref->client->Con_Printf(PRINT_ALL, text);
}

#endif