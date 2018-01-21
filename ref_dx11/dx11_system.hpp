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

#ifndef __DX11_SYSTEM_HPP__
#define __DX11_SYSTEM_HPP__
#pragma once

#define	WINDOW_CLASS_NAME	"Quake 2"

#include "dx11_local.hpp"

namespace dx11
{
	class System 
	{
		friend class Dx;
	private:
		HINSTANCE					m_hInstance;
		WNDPROC						m_wndProc;
		WNDCLASSEX					m_wndClassEx;
		HWND						m_hWnd;

		bool						m_inRegistration;
		bool						m_uploadBatchOpen;

		bool						m_clockRunning;
		bool						m_clockFrequencyObtained;
		LARGE_INTEGER				m_clockFrequency;

		bool						VID_CreateWindow();
		void						VID_DestroyWindow();

	public:
		std::unique_ptr<Dx>			dx;			// Backend
		std::unique_ptr<Web>		web;		// Networking

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertUTF;

									System();

		bool						Initialize(HINSTANCE hInstance, WNDPROC wndProc);
		void						Shutdown();

		void						AppActivate(bool active);

		void						BeginRegistration();
		void						EndRegistration();

		void						BeginUpload();
		void						EndUpload();

		std::string					GetCurrentWorkingDirectory();
		bool						SetCurrentWorkingDirectory(std::string directory);
	};
}

#endif // !__DX11_SYSTEM_HPP__