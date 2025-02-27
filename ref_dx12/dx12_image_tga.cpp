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
2025 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

typedef struct _TargaHeader {
	unsigned char 	id_length = 0, colormap_type = 0, image_type = 0;
	unsigned short	colormap_index = 0, colormap_length = 0;
	unsigned char	colormap_size = 0;
	unsigned short	x_origin = 0, y_origin = 0, width = 0, height = 0;
	unsigned char	pixel_size = 0, attributes = 0;
} TargaHeader;


/*
================
LoadTga
================
*/
void dx12::ImageManager::LoadTGA(byte* raw, size_t bufferLength, byte** pic, UINT64& width, unsigned int& height)
{
	LOG_FUNC();

	try
	{
		int		columns = 0, rows = 0;
		size_t numPixels = 0;
		byte* pixbuf = nullptr;
		int		row = 0, column = 0;
		byte* buf_p = nullptr;
		TargaHeader		targa_header = {};
		byte* targa_rgba = nullptr;
		byte tmp[2] = {};

		*pic = nullptr;

		buf_p = raw;

		targa_header.id_length = *buf_p++;
		targa_header.colormap_type = *buf_p++;
		targa_header.image_type = *buf_p++;

		tmp[0] = buf_p[0];
		tmp[1] = buf_p[1];
        targa_header.colormap_index = LittleUShort(*reinterpret_cast<unsigned short*>(tmp));
		buf_p += 2;
		tmp[0] = buf_p[0];
		tmp[1] = buf_p[1];
		targa_header.colormap_length = LittleUShort(*reinterpret_cast<unsigned short*>(tmp));
		buf_p += 2;
		targa_header.colormap_size = *buf_p++;
		targa_header.x_origin = LittleUShort(*reinterpret_cast<unsigned short*>(buf_p));
		buf_p += 2;
		targa_header.y_origin = LittleUShort(*reinterpret_cast<unsigned short*>(buf_p));
		buf_p += 2;
		targa_header.width = LittleUShort(*reinterpret_cast<unsigned short*>(buf_p));
		buf_p += 2;
		targa_header.height = LittleUShort(*reinterpret_cast<unsigned short*>(buf_p));
		buf_p += 2;
		targa_header.pixel_size = *buf_p++;
		targa_header.attributes = *buf_p++;

		if (targa_header.image_type != 2
			&& targa_header.image_type != 10)
		{
			ref->client->Sys_Error(ERR_DROP, L"LoadTGA: Only type 2 and 10 targa RGB images supported\n");
		}

		if (targa_header.colormap_type != 0
			|| (targa_header.pixel_size != 32 && targa_header.pixel_size != 24))
		{
			ref->client->Sys_Error(ERR_DROP, L"LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		}

		columns = targa_header.width;
		rows = targa_header.height;
		numPixels = static_cast<size_t>(columns) * rows;

		width = columns;
		height = rows;

		targa_rgba = new byte[numPixels * 4];
		*pic = targa_rgba;

		if (targa_header.id_length != 0)
			buf_p += targa_header.id_length;  // skip TARGA image comment

		if (targa_header.image_type == 2) {  // Uncompressed, RGB images
			for (row = rows - 1; row >= 0; row--) {
				pixbuf = targa_rgba + row * columns * 4;
				for (column = 0; column < columns; column++) {
					unsigned char red, green, blue, alphabyte;
					switch (targa_header.pixel_size) {
					case 24:

						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						break;
					}
				}
			}
		}
		else if (targa_header.image_type == 10) {   // Runlength encoded RGB images
			unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;
			for (row = rows - 1; row >= 0; row--) {
				pixbuf = targa_rgba + row * columns * 4;
				for (column = 0; column < columns; ) {
					packetHeader = *buf_p++;
					packetSize = 1 + (packetHeader & 0x7f);
					if (packetHeader & 0x80) {        // run-length packet
						switch (targa_header.pixel_size) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							break;
						}

						for (j = 0; j < packetSize; j++) {
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							column++;
							if (column == columns) { // run spans across rows
								column = 0;
								if (row > 0)
									row--;
								else
									goto breakOut;
								pixbuf = targa_rgba + row * columns * 4;
							}
						}
					}
					else {                            // non run-length packet
						for (j = 0; j < packetSize; j++) {
							switch (targa_header.pixel_size) {
							case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = 255;
								break;
							case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = alphabyte;
								break;
							}
							column++;
							if (column == columns) { // pixel packet run spans across rows
								column = 0;
								if (row > 0)
									row--;
								else
									goto breakOut;
								pixbuf = targa_rgba + row * columns * 4;
							}
						}
					}
				}
			breakOut:;
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}