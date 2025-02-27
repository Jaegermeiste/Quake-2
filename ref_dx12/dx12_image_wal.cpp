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
/*
================
LoadWal
================
*/
void dx12::ImageManager::LoadWAL(byte* raw, size_t bufferLength, byte** pic, size_t& width, size_t& height)
{
	LOG_FUNC();

	try
	{
		miptex_t* miptex = reinterpret_cast<miptex_t*>(raw);
		size_t	  offset = 0;

		if (!miptex)
		{
			*pic = nullptr;
			return;
		}

		width = LittleULong(miptex->width);
		height = LittleULong(miptex->height);
		offset = LittleULong(miptex->offsets[0]);

		// WAL is effectively directly usable
		*pic = (reinterpret_cast<byte*>(miptex) + offset);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}