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

dx11::System::System()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	m_hInstance = nullptr;
	m_wndProc = nullptr;
	ZeroMemory(&m_wndClassEx, sizeof(WNDCLASS));
	m_hWnd = nullptr;

	// Timing
	if (QueryPerformanceFrequency(&m_clockFrequency) == TRUE)
	{
		m_clockFrequencyObtained = true;
	}
	else
	{
		m_clockFrequencyObtained = false;
		ref->client->Sys_Error(ERR_FATAL, "Couldn't obtain clock frequency.");
	}

	dx = std::make_unique<Dx>();
	web = std::make_unique<Web>();
}

void dx11::System::BeginRegistration()
{
	LOG_FUNC();

	if (!m_inRegistration)
	{
		BeginUpload();

		m_inRegistration = true;
	}
}

void dx11::System::EndRegistration()
{
	LOG_FUNC();

	if (m_inRegistration)
	{
		m_inRegistration = false;

		EndUpload();
	}
}

void dx11::System::BeginUpload()
{
	LOG_FUNC();

	/*if (resourceUpload == nullptr)
	{
		resourceUpload = new DirectX::ResourceUploadBatch(ref->sys->d3dDevice);
	}*/

	if (!m_uploadBatchOpen)
	{
		//resourceUpload->Begin();

		m_uploadBatchOpen = true;
	}
}

void dx11::System::EndUpload()
{
	LOG_FUNC();

	// Only flush the upload if the batch is open AND we are not in registration mode
	if (m_uploadBatchOpen && (!m_inRegistration))
	{
		// Upload the resources to the GPU.
		//auto uploadResourcesFinished = resourceUpload->End(ref->sys->cmdQueue);

		// Wait for the upload thread to terminate
		//uploadResourcesFinished.wait();

		m_uploadBatchOpen = false;
	}
}

/*
** VID_CreateWindow
*/
bool dx11::System::VID_CreateWindow()
{
	LOG_FUNC();

	RECT				r			= {};
	ZeroMemory(&r, sizeof(RECT));
	DWORD				exstyle		= 0,
						stylebits	= 0;
	int					x			= 0,
						y			= 0,
						w			= 0,
						h			= 0;
	
	const LONG			width		= ref->cvars->r_customWidth->Int();
	const LONG			height		= ref->cvars->r_customHeight->Int();
	const bool			fullscreen	= ref->cvars->vid_fullscreen->Bool();

	ref->client->Con_Printf(PRINT_ALL, "Creating window");

	/* Register the frame class */
	m_wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	m_wndClassEx.lpfnWndProc = m_wndProc;
	m_wndClassEx.cbClsExtra = 0;
	m_wndClassEx.cbWndExtra = 0;
	m_wndClassEx.hInstance = m_hInstance;
	m_wndClassEx.hIcon = (HICON)LoadImage(m_hInstance, "q2.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	m_wndClassEx.hIconSm = (HICON)LoadImage(m_hInstance, "q2.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	m_wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	//m_wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_GRAYTEXT + 1);
	m_wndClassEx.lpszMenuName = nullptr;
	m_wndClassEx.lpszClassName = WINDOW_CLASS_NAME;
	m_wndClassEx.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&m_wndClassEx))
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't register window class");
	}

	// Note that adding the sysmenu and hitting close puts the game thread into an infinite loop, so don't do it
	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP | WS_VISIBLE; //| WS_SYSMENU;
	}
	else
	{
		exstyle = 0;
		stylebits = WINDOW_STYLE; //| WS_SYSMENU;
	}

	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;

	AdjustWindowRect(&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		x = ref->cvars->vid_xPos->Int();
		y = ref->cvars->vid_yPos->Int();
	}

	m_hWnd = CreateWindowEx(
		exstyle,
		WINDOW_CLASS_NAME,
		"Quake 2",
		stylebits,
		x, y, w, h,
		NULL,
		NULL,
		m_hInstance,
		nullptr);

	if (!m_hWnd)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't create window");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	// let the sound and input subsystems know about the new window
	dx11::ref->client->Vid_NewWindow(msl::utilities::SafeInt<unsigned int>(width), msl::utilities::SafeInt<unsigned int>(height));

	return true;
}

void dx11::System::VID_DestroyWindow()
{
	LOG_FUNC();
	
	if (m_hWnd != nullptr)
	{
		ref->client->Con_Printf(PRINT_ALL, "...destroying window\n");

		ShowWindow(m_hWnd, SW_SHOWNORMAL);	// prevents leaving empty slots in the taskbar
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		UnregisterClass(WINDOW_CLASS_NAME, m_hInstance);
	}
}


bool dx11::System::Initialize(HINSTANCE hInstance, WNDPROC wndProc)
{
	LOG_FUNC();

	LOG(info) << "Starting up.";

	Shutdown();

	m_hInstance = hInstance;
	m_wndProc = wndProc;

	if (!VID_CreateWindow())
	{
		LOG(error) << "Failed to create window";
		return false;
	}

	if ((!dx) || (!dx->Initialize(m_hWnd)))
	{
		LOG(error) << "Failed to create DirectX system";
		return false;
	}

	if ((!web) || (!web->Initialize()))
	{
		LOG(error) << "Failed to create network system";
		return false;
	}

	return true;
}

std::string dx11::System::GetCurrentWorkingDirectory()
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

bool dx11::System::SetCurrentWorkingDirectory(std::string directory)
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

void dx11::System::Shutdown()
{
	LOG_FUNC();

	if (web)
	{
		web->Shutdown();
	}

	if (dx)
	{
		dx->Shutdown();
	}

	if (m_hWnd != nullptr)
	{
		VID_DestroyWindow();
	}
}

void dx11::System::AppActivate(bool active)
{
	LOG_FUNC();

	if (active)
	{
		LOG(info) << "Restoring Window.";

		SetForegroundWindow(m_hWnd);
		ShowWindow(m_hWnd, SW_RESTORE);
	}
	else
	{
		if (ref->cvars->vid_fullscreen->Bool())
		{
			LOG(info) << "Minimizing Window.";

			ShowWindow(m_hWnd, SW_MINIMIZE);
		}
	}
}

bool dx11::System::DoesFileExist(std::string fileName)
{
	// Courtesy https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program
	DWORD dwAttrib = GetFileAttributes(fileName.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring dx11::System::ToWideString(std::string inStr)
{
	// Courtesy https://stackoverflow.com/questions/6693010/problem-using-multibytetowidechar
	// Courtesy https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
	if (inStr.empty())
	{
		return std::wstring();
	}

	int strLen = MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), msl::utilities::SafeInt<int>(inStr.size()), NULL, 0);
	std::wstring outWString(msl::utilities::SafeInt<size_t>(strLen + 1), 0);
	MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), msl::utilities::SafeInt<int>(inStr.size()), &outWString[0], strLen);
	
	return outWString;
}

std::string dx11::System::ToString(WCHAR* inWideStr)
{
	// Courtesy https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
	if (!inWideStr)
	{
		return std::string();
	}

	int strLen = WideCharToMultiByte(CP_UTF8, 0, inWideStr, msl::utilities::SafeInt<int>(wcslen(inWideStr)), NULL, 0, NULL, NULL);
	std::string outString(msl::utilities::SafeInt<size_t>(strLen + 1), 0);
	WideCharToMultiByte(CP_UTF8, 0, inWideStr, msl::utilities::SafeInt<int>(wcslen(inWideStr)), &outString[0], strLen, NULL, NULL);
	return outString;
}
