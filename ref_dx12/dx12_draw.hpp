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

#ifndef __DX12_DRAW_HPP__
#define __DX12_DRAW_HPP__
#pragma once

#include "dx12_local.hpp"

constexpr auto NUM_INITIAL_DRAW_QUADS = 2048;

namespace dx12
{
	class Draw {
	private:
		std::vector<Quad2D> m_quadList = std::vector<Quad2D>(NUM_INITIAL_DRAW_QUADS);
		unsigned int m_quadIndex = 0;

	public:
		void		GetPicSize	(unsigned int &w, unsigned int &h, std::wstring name);
		void		Pic			(int x, int y, std::wstring name);
		void		StretchPic	(int x, int y, int w, int h, std::wstring name);
		void		Char		(int x, int y, unsigned char c);
		void		TileClear	(int x, int y, int w, int h, std::wstring name);
		void		Fill		(int x, int y, int w, int h, int c);
		void		FadeScreen	(void);
		void		StretchRaw	(int x, int y, int w, int h, unsigned int cols, unsigned int rows, byte *data);

		void        BeginFrame() { m_quadIndex = 0; };
		void        EndFrame() { return; };

		void        Initialize();
		void        Shutdown();
	};
}

#endif // !__DX12_DRAW_HPP__