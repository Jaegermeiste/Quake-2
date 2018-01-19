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

std::string dx11::GetCurrentWorkingDirectory()
{
	LOG_FUNC();

	TCHAR currentWorkingDirectory[MAX_PATH + 1];
	ZeroMemory(&currentWorkingDirectory, (sizeof(TCHAR) * MAX_PATH) + sizeof(TCHAR));

	// Request ownership of the critical section.
	LOG(trace) << "Entering critical section";
	EnterCriticalSection(&CriticalSection);
	LOG(trace) << "Entered critical section";

	// Get current working directory
	DWORD dwRet = GetCurrentDirectory(MAX_PATH, currentWorkingDirectory);

	if (dwRet == 0)
	{
		LOG(error) << "GetCurrentDirectory failed " << GetLastError();
		return std::string();
	}
	if (dwRet > MAX_PATH)
	{
		LOG(warning) << "Buffer too small; need " << dwRet << " characters, terminating at MAX_PATH=" << MAX_PATH;
		currentWorkingDirectory[MAX_PATH] = 0;
	}

	LOG(info) << "Current working directory is " << currentWorkingDirectory;

	// Release ownership of the critical section.
	LOG(trace) << "Leaving critical section";
	LeaveCriticalSection(&CriticalSection);
	LOG(trace) << "Left critical section";

	return currentWorkingDirectory;
}

bool dx11::SetCurrentWorkingDirectory(std::string directory)
{
	LOG_FUNC();

	std::string currentWorkingDirectory = GetCurrentWorkingDirectory();

	if (directory != currentWorkingDirectory)
	{
		// Request ownership of the critical section.
		LOG(trace) << "Entering critical section";
		EnterCriticalSection(&CriticalSection);
		LOG(trace) << "Entered critical section";

		LOG(info) << "Changing current working directory to: " << directory;

		BOOL success = SetCurrentDirectory(directory.c_str());

		// Release ownership of the critical section.
		LOG(trace) << "Leaving critical section";
		LeaveCriticalSection(&CriticalSection);
		LOG(trace) << "Left critical section";

		if (!success)
		{
			LOG(error) << "SetCurrentDirectory failed " << GetLastError();
			return false;
		}

		currentWorkingDirectory = GetCurrentWorkingDirectory();

		if (directory == currentWorkingDirectory)
		{
			LOG(info) << "Current working directory is now " << currentWorkingDirectory;
		}
		else
		{
			LOG(error) << "Current working directory is " << currentWorkingDirectory << ", which is not what was requested: " << directory;
		}
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