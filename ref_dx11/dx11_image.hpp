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

#define BPP_8	8
#define BPP_24	24
#define BPP_32	32

namespace dx11
{
	class Texture
	{
		friend class ImageManager;
	public:
		std::string				m_name;
		D3D11_TEXTURE2D_DESC	m_textureDesc;
		D3D11_SUBRESOURCE_DATA	m_data;
		ID3D11Texture2D*		m_texture2D = nullptr;
	};

	class ImageManager {
	private:
		unsigned int	m_8to24table[256];

		std::map<std::shared_ptr<image_t>, Microsoft::WRL::ComPtr<ID3D11Resource>> m_images;

		void GetPalette(void);
		static void LoadWal(std::string fileName, byte **pic, unsigned int &width, unsigned int &height);
		static void LoadPCX(std::string fileName, byte **pic, byte **palette, unsigned int &width, unsigned int &height);

		Texture*	CreateTexture2DFromRaw(ID3D11Device* m_d3dDevice, std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte** raw);

		void UploadScratchImage(DirectX::ScratchImage & image, ID3D11Resource** pResource, bool generateMipMap);

		bool		UploadTexture2D(Texture *texture);
	public:
		std::shared_ptr<image_t>	Load(std::string name, imagetype_t type);
	};

	
}

#endif // !__DX11_IMAGE_HPP__