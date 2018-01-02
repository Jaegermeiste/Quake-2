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

#ifndef __DX11_IMAGE_HPP__
#define __DX11_IMAGE_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class Image {
	private:
		struct Texture2D
		{
			unsigned int	width;
			unsigned int	height;
			size_t			size;
			DXGI_FORMAT		format;
			unsigned int	*data;
		};

		unsigned int	d_8to24table[256];

		std::map<std::shared_ptr<image_t>, Microsoft::WRL::ComPtr<ID3D11Resource>> images;

		void GetPalette(void);
		static void LoadWal(std::string fileName, byte **pic, unsigned int &width, unsigned int &height);
		static void LoadPCX(std::string fileName, byte **pic, byte **palette, unsigned int &width, unsigned int &height);

		Texture2D*	CreateTexture2DFromRaw(unsigned int width, unsigned int height, byte** raw);

		void UploadScratchImage(DirectX::ScratchImage & image, ID3D11Resource** pResource, bool generateMipMap);

		bool		UploadTexture2D(Texture2D *texture);
	public:
		std::shared_ptr<image_t>	Load(std::string name, imagetype_t type);
	};
}

#endif // !__DX11_IMAGE_HPP__