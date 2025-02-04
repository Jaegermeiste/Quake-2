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
20179Bleeding Eye Studios
*/

#include "dx12_local.hpp"

dx12::System::System()
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
		ref->client->Sys_Error(ERR_FATAL, L"Couldn't obtain clock frequency.");
	}

	dx = std::make_unique<Dx>();
	web = std::make_unique<Web>();
}

/*
** VID_CreateWindow
*/
bool dx12::System::VID_CreateWindow()
{
	LOG_FUNC();

	RECT				r = {};
	ZeroMemory(&r, sizeof(RECT));
	DWORD				exstyle = 0,
		stylebits = 0;
	int					x = 0,
		y = 0,
		w = 0,
		h = 0;

	const LONG			width = ref->cvars->r_customWidth->Int();
	const LONG			height = ref->cvars->r_customHeight->Int();
	const bool			fullscreen = ref->cvars->vid_fullscreen->Bool();

	ref->client->Con_Printf(PRINT_ALL, L"Creating window");

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
		ref->client->Sys_Error(ERR_FATAL, L"Couldn't register window class");
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
		ref->client->Sys_Error(ERR_FATAL, L"Couldn't create window");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	// let the sound and input subsystems know about the new window
	dx12::ref->client->Vid_NewWindow(msl::utilities::SafeInt<unsigned int>(width), msl::utilities::SafeInt<unsigned int>(height));

	return true;
}

void dx12::System::VID_DestroyWindow()
{
	LOG_FUNC();

	if (m_hWnd != nullptr)
	{
		ref->client->Con_Printf(PRINT_ALL, L"...destroying window\n");

		ShowWindow(m_hWnd, SW_SHOWNORMAL);	// prevents leaving empty slots in the taskbar
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		UnregisterClass(WINDOW_CLASS_NAME, m_hInstance);
	}
}


bool dx12::System::Initialize(HINSTANCE hInstance, WNDPROC wndProc)
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

std::wstring dx12::System::GetCurrentWorkingDirectory()
{
	LOG_FUNC();

	WCHAR currentWorkingDirectory[MAX_PATH + 1];
	ZeroMemory(&currentWorkingDirectory, (sizeof(TCHAR) * MAX_PATH) + sizeof(TCHAR));

	// Request ownership of the critical section.
	LOG(trace) << "Entering critical section";
	EnterCriticalSection(&CriticalSection);
	LOG(trace) << "Entered critical section";

	// Get current working directory
	DWORD dwRet = GetCurrentDirectoryW(MAX_PATH, currentWorkingDirectory);

	if (dwRet == 0)
	{
		LOG(error) << "GetCurrentDirectory failed " << GetLastError();
		return std::wstring();
	}
	if (dwRet > MAX_PATH)
	{
		LOG(warning) << "Buffer too small; need " << dwRet << " characters, terminating at MAX_PATH=" << MAX_PATH;
		currentWorkingDirectory[MAX_PATH] = 0;
	}

	LOG(info) << "Current working directory is " << ref->sys->ToWideString(currentWorkingDirectory);

	// Release ownership of the critical section.
	LOG(trace) << "Leaving critical section";
	LeaveCriticalSection(&CriticalSection);
	LOG(trace) << "Left critical section";

	return currentWorkingDirectory;
}

bool dx12::System::SetCurrentWorkingDirectory(std::wstring directory)
{
	LOG_FUNC();

	std::wstring currentWorkingDirectory = GetCurrentWorkingDirectory();

	if (directory != currentWorkingDirectory)
	{
		// Request ownership of the critical section.
		LOG(trace) << "Entering critical section";
		EnterCriticalSection(&CriticalSection);
		LOG(trace) << "Entered critical section";

		LOG(info) << "Changing current working directory to: " << directory;

		BOOL success = SetCurrentDirectoryW(directory.c_str());

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

void dx12::System::Shutdown()
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

void dx12::System::AppActivate(bool active)
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

bool dx12::System::DoesFileExist(std::wstring fileName)
{
	// Courtesy https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program
	DWORD dwAttrib = GetFileAttributesW(fileName.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring dx12::System::ToWideString(std::string inStr)
{
	if (inStr.empty())
	{
		return std::wstring();
	}

	return boost::locale::conv::utf_to_utf<wchar_t>(inStr);
}

std::wstring dx12::System::ToWideString(WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::wstring();
	}

	return std::wstring(inWideStr);
}

std::wstring dx12::System::ToWideString(const WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::wstring();
	}

	return std::wstring(inWideStr);
}

std::string dx12::System::ToString(std::wstring inWideStr)
{
	if (inWideStr.empty())
	{
		return std::string();
	}

	return boost::locale::conv::utf_to_utf<char>(inWideStr);
}

std::string dx12::System::ToString(WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::string();
	}

	return boost::locale::conv::utf_to_utf<char>(inWideStr);
}

Vector2 dx12::System::GetNormalizedDeviceCoordinates(int px, int py, int windowWidth, int windowHeight)
{
	Vector2 v = {};

	// Convert pixel coordinates to NDC (-1 to 1 range)
	v.x = (2.0f * px / windowWidth) - 1.0f;   // Convert x
	v.y = 1.0f - (2.0f * py / windowHeight);  // Convert y

	return v;
}

Vector4 dx12::System::GetNormalizedDeviceRectangle(int px, int py, int pw, int ph, int windowWidth, int windowHeight)
{
	Vector4 v = {};

	// Convert pixel coordinates to NDC (-1 to 1 range)
	v.x = (2.0f * px / windowWidth) - 1.0f;
	v.y = 1.0f - (2.0f * py / windowHeight);
	v.z = 2.0f * pw / windowWidth;
	v.w = 2.0f * ph / windowHeight;

	return v;
}
