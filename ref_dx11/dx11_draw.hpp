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

#ifndef __DX11_DRAW_HPP__
#define __DX11_DRAW_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class Draw {
	public:
		void		GetPicSize	(unsigned int &w, unsigned int &h, std::string name);
		void		Pic			(int x, int y, std::string name);
		void		StretchPic	(int x, int y, int w, int h, std::string name);
		void		Char		(int x, int y, unsigned int c);
		void		TileClear	(int x, int y, int w, int h, std::string name);
		void		Fill		(int x, int y, int w, int h, int c);
		void		FadeScreen	(void);
		void		StretchRaw	(int x, int y, int w, int h, unsigned int cols, unsigned int rows, byte *data);
	};
}

#endif // !__DX11_DRAW_HPP__