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

#ifndef __DX12_SYSTEM_HPP__
#define __DX12_SYSTEM_HPP__
#pragma once

constexpr auto	WINDOW_CLASS_NAME =	"Quake 2";

#include "dx12_local.hpp"

namespace dx12
{
	class System
	{
		friend class Dx;
	private:
		HINSTANCE					m_hInstance;
		WNDPROC						m_wndProc;
		WNDCLASSEX					m_wndClassEx;
		HWND						m_hWnd;

		bool						m_clockRunning;
		bool						m_clockFrequencyObtained;
		LARGE_INTEGER				m_clockFrequency;

		bool						VID_CreateWindow();
		void						VID_DestroyWindow();

		boost::uuids::time_generator_v7 uuid7_gen;

	public:
		std::unique_ptr<Dx>			dx;			// Backend
		std::unique_ptr<Web>		web;		// Networking

		System();

		bool						Initialize(HINSTANCE hInstance, WNDPROC wndProc);
		void						Shutdown();

		void						AppActivate(bool active);

		std::wstring				GetCurrentWorkingDirectory();
		bool						SetCurrentWorkingDirectory(std::wstring directory);

		bool						DoesFileExist(std::wstring fileName);

		std::wstring				ToWideString(std::string inStr);
		std::wstring				ToWideString(WCHAR* inWideStr);
		std::wstring				ToWideString(const WCHAR* inWideStr);
		std::string					ToString(std::wstring inStr);
		std::string					ToString(WCHAR* inWideStr);

		std::wstring                GetUUIDv7() { return boost::uuids::to_wstring(uuid7_gen()); };

		Vector2                     GetNormalizedDeviceCoordinates(int px, int py, int windowWidth, int windowHeight);
		Vector4                     GetNormalizedDeviceRectangle (int px, int py, int pw, int ph, int windowWidth, int windowHeight);
	};
};

#endif // !__DX12_SYSTEM_HPP__