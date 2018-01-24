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

#ifndef __DX11_SUBSYSTEMTEXT_HPP__
#define __DX11_SUBSYSTEMTEXT_HPP__
#pragma once

#include "dx11_local.hpp"

#define CHAR_SIZE	8

namespace dx11
{
	__declspec(align(16)) class SubsystemText {
		friend class System;
	private:
		IDWriteFactory*				m_writeFactory = nullptr;
		IDWriteTextFormat*			m_textFormat = nullptr;

		byte						m_padding[8];

	public:
									SubsystemText();

		bool						Initialize();

		void						RenderText(int x, int y, int w, int h, std::string text, ID2D1SolidColorBrush* colorBrush);
		void						RenderText(int x, int y, int w, int h, WCHAR* text, ID2D1SolidColorBrush* colorBrush);

		void						Shutdown();

		//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
}

#endif // !__DX11_SUBSYSTEMTEXT_HPP__