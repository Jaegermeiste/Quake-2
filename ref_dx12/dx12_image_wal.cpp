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
void dx12::ImageManager::LoadWal(std::wstring fileName, byte **pic, UINT64 &width, unsigned int &height)
{
	LOG_FUNC();

	miptex_t		*mt = nullptr;
	unsigned int	ofs = 0;

	ref->client->FS_LoadFile(fileName, (void **)&mt);

	if (!mt)
	{
		ref->client->Con_Printf(PRINT_ALL, L"LoadWal: can't load " + fileName + L"\n");
		return;
	}

	width = LittleULong(mt->width);
	height = LittleULong(mt->height);
	ofs = LittleULong(mt->offsets[0]);

	//image = GL_LoadPic(name, (byte *)mt + ofs, m_width, m_height, it_wall, 8);

	ref->client->FS_FreeFile((void *)mt);
}