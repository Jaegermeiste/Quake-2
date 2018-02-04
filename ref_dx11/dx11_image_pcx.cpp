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

/*
==============
LoadPCX
==============
*/
void dx11::ImageManager::LoadPCX(byte* raw, int len, byte **pic, byte **palette, unsigned int &width, unsigned int &height)
{
	LOG_FUNC();

	pcx_t	*pcx = nullptr;
	byte	dataByte = 0;
	int		runLength = 0;
	byte	*out = nullptr,
		*pix = nullptr;

	*pic = nullptr;
	*palette = nullptr;
	width = 0;
	height = 0;

	if ((!raw) || (len <= 0))
	{
		LOG(warning) << "Empty buffer passed.";
		return;
	}

	//
	// parse the PCX file
	//
	pcx = reinterpret_cast<pcx_t *>(raw);

	pcx->xmin = LittleUShort(pcx->xmin);
	pcx->ymin = LittleUShort(pcx->ymin);
	pcx->xmax = LittleUShort(pcx->xmax);
	pcx->ymax = LittleUShort(pcx->ymax);
	pcx->hres = LittleUShort(pcx->hres);
	pcx->vres = LittleUShort(pcx->vres);
	pcx->bytes_per_line = LittleUShort(pcx->bytes_per_line);
	pcx->palette_type = LittleUShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ref->client->Con_Printf(PRINT_ALL, "Bad PCX file.");
		return;
	}

	out = new byte[(pcx->ymax + 1u) * (pcx->xmax + 1u)]();

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = new byte[768]();
		memcpy(*palette, reinterpret_cast<byte *>(pcx) + len - 768, 768);
	}

	width = msl::utilities::SafeInt<unsigned int>(pcx->xmax + 1);
	height = msl::utilities::SafeInt<unsigned int>(pcx->ymax + 1);

	for (unsigned short y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1)
	{
		for (unsigned short x = 0; x <= pcx->xmax; )
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - reinterpret_cast<byte *>(pcx) > len)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "PCX file was malformed.");
		delete[] * pic;
		*pic = nullptr;
	}
}