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
2019 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

void dx12::Client::Sys_Error(unsigned short err_level, std::wstring str)
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

	m_refImport.Sys_Error(err_level, const_cast<char*>(ref->sys->ToString(str).c_str()));
}

void dx12::Client::Cmd_AddCommand(std::wstring name, void(*cmd)())
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<name> " << name << " <cmd> " << cmd;

	char *cName = new char[64]();
	strncpy_s(cName, 64, ref->sys->ToString(name).c_str(), name.length());
	m_cmdNames.push_back(cName);

	m_refImport.Cmd_AddCommand(m_cmdNames.back(), cmd);
}

void dx12::Client::Cmd_RemoveCommand(std::wstring name)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<name> " << name;

	m_refImport.Cmd_RemoveCommand(const_cast<char*>(ref->sys->ToString(name).c_str()));
}

unsigned int dx12::Client::Cmd_Argc(void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return msl::utilities::SafeInt<unsigned int>(m_refImport.Cmd_Argc());
}

std::wstring dx12::Client::Cmd_Argv(unsigned int i)
{
	LOG_FUNC();

	int clientIndex = msl::utilities::SafeInt<int>(i);

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return ref->sys->ToWideString(m_refImport.Cmd_Argv(clientIndex));
}

void dx12::Client::Cmd_ExecuteText(unsigned int exec_when, std::wstring text)
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

	m_refImport.Cmd_ExecuteText(clientWhen, const_cast<char*>(ref->sys->ToString(text).c_str()));
}

void dx12::Client::Con_Printf(unsigned short print_level, std::wstring str)
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

	str += L"\n";

	m_refImport.Con_Printf(print_level, const_cast<char*>(ref->sys->ToString(str).c_str()));
}

int dx12::Client::FS_LoadFile(std::wstring fileName, void **buf)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(trace) << "<fileName> " << fileName << " <buf> " << buf;

	int retVal = m_refImport.FS_LoadFile(const_cast<char*>(ref->sys->ToString(fileName).c_str()), buf);

	if (retVal > 0)
	{
		LOG(info) << "Loaded " << fileName << " with length " << retVal << " at address " << *buf;
	}

	return retVal;
}

void dx12::Client::FS_FreeFile(void *buf)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	LOG(info) << "<buf> " << buf;

	m_refImport.FS_FreeFile(buf);
}

std::wstring dx12::Client::FS_Gamedir(void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	return ref->sys->ToWideString(m_refImport.FS_Gamedir());
}

std::wstring dx12::Client::FS_GamedirAbsolute(void)
{
	LOG_FUNC();

	WCHAR  absoluteGamedirBuffer[4096] = TEXT(L"");
	ZeroMemory(&absoluteGamedirBuffer, sizeof(WCHAR) * 4096);

	std::wstring gameDir = FS_Gamedir();

	// Request ownership of the critical section.
	LOG(trace) << "Entering critical section";
	EnterCriticalSection(&CriticalSection);
	LOG(trace) << "Entered critical section";

	DWORD  retval = GetFullPathNameW(gameDir.c_str(), MAX_PATH, absoluteGamedirBuffer, NULL);

	// Release ownership of the critical section.
	LOG(trace) << "Leaving critical section";
	LeaveCriticalSection(&CriticalSection);
	LOG(trace) << "Left critical section";

	if (retval == 0)
	{
		LOG(error) << "GetFullPathName failed: " << GetLastError();
		return std::wstring();
	}
	else
	{
		LOG(info) << "The full path name is: " << ref->sys->ToWideString(absoluteGamedirBuffer);
	}

	return absoluteGamedirBuffer;
}

//cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
//cvar_t			*(*Cvar_Set)			(char *name, char *value);
//void(*Cvar_SetValue)		(char *name, float value);

bool dx12::Client::Vid_GetModeInfo(unsigned int &width, unsigned int &height, int mode)
{
	LOG_FUNC();

	int clientWidth = 0,
		clientHeight = 0;

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	qboolean retVal = m_refImport.Vid_GetModeInfo(&clientWidth, &clientHeight, mode);

	LOG(info) << "<width> " << width << " <height> " << height << " <mode> " << mode << " {return value} " << retVal;

	if (retVal == true)
	{
		width = msl::utilities::SafeInt<int>(clientWidth);
		height = msl::utilities::SafeInt<int>(clientHeight);
		return true;
	}
	return false;
}

void dx12::Client::Vid_MenuInit(void)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	m_refImport.Vid_MenuInit();
}

void dx12::Client::Vid_NewWindow(unsigned int width, unsigned int height)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> guard(m_refImportMutex);

	m_refImport.Vid_NewWindow(msl::utilities::SafeInt<int>(width), msl::utilities::SafeInt<int>(height));
}

void dx12::Client::SetRefImport(refimport_t rimp)
{
	LOG_FUNC();

	m_refImport = rimp;

	// Pass everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cvar_Get = m_refImport.Cvar_Get;
	dx12::Client::Cvar_Set = m_refImport.Cvar_Set;
	dx12::Client::Cvar_SetValue = m_refImport.Cvar_SetValue;
};

dx12::Client::Client(refimport_t rimp)
{
	LOG_FUNC();

	SetRefImport(rimp);
}

dx12::Client::~Client()
{
	// Null everything through that isn't explicitly overridden in the class functions above
	dx12::Client::Cvar_Get = nullptr;
	dx12::Client::Cvar_Set = nullptr;
	dx12::Client::Cvar_SetValue = nullptr;

	// Blow out any stored cmdnames
	for (auto & cmdName : m_cmdNames)
	{
		if (cmdName)
		{
			delete[] cmdName;
			cmdName = nullptr;
		}
	}
}

#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error(char *error, ...)
{
	va_list		argptr;
	wchar_t		text[1024];

	va_start(argptr, error);
	vswprintf(text, 1024, dx12::ref->sys->ToWideString(error).c_str(), argptr);
	va_end(argptr);

	dx12::ref->client->Sys_Error(ERR_FATAL, text);
}

void Com_Printf(char *fmt, ...)
{
	va_list		argptr;
	wchar_t		text[1024];

	va_start(argptr, fmt);
	vswprintf(text, 1024, dx12::ref->sys->ToWideString(fmt).c_str(), argptr);
	va_end(argptr);

	dx12::ref->client->Con_Printf(PRINT_ALL, text);
}

#endif