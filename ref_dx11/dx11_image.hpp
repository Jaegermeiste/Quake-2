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
	class Texture2D
	{
		friend class ImageManager;
	public:
		class Texture2DData
		{
			friend class ImageManager;
		public:
			D3D11_TEXTURE2D_DESC		m_textureDesc;
			ID3D11ShaderResourceView*	m_shaderResourceView = nullptr;
			ID3D11Resource*				m_resource = nullptr;
			ID3D11Texture2D*			m_texture2D = nullptr;
			imagetype_t					m_imageType;
			std::string					m_format;
		};

		qhandle_t						m_handle;
		std::string						m_name;
		unsigned int					m_registrationSequence = 0;
		std::shared_ptr<Texture2DData>	m_data;
	};



	class ImageManager
	{
		friend class Draw;
	private:
		std::vector<std::string> m_imageExtensions =
		{
			"dds", "exr", "hdr", "tga", "png", "jpg", "tif", "gif", "bmp", "ico", "wdp", "jxr", "wal", "pcx"
		};

		qhandle_t				m_lastHandle = 0;

		//std::map<std::string, std::shared_ptr<Texture2D>> m_images;

		typedef boost::multi_index_container<
			Texture2D,

			boost::multi_index::indexed_by<
			// Enable random access
			boost::multi_index::random_access<>,

			// sort by handle
			boost::multi_index::hashed_unique<boost::multi_index::tag<handle>, boost::multi_index::member<Texture2D, qhandle_t, &Texture2D::m_handle> >,

			// sort by name
			boost::multi_index::hashed_unique<boost::multi_index::tag<name>, boost::multi_index::member<Texture2D, std::string, &Texture2D::m_name> >,

			// sort by registration sequence
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<registration>, boost::multi_index::member<Texture2D, unsigned int, &Texture2D::m_registrationSequence> >
			>
		> ImageContainer;
		typedef ImageContainer::index<handle>::type			ImagesByHandle;
		typedef ImageContainer::index<name>::type			ImagesByName;
		typedef ImageContainer::index<registration>::type	ImagesByRegistrationSequence;
		
		ImageContainer										m_images;
		ImagesByHandle										m_imagesByHandle;
		ImagesByName										m_imagesByName;
		ImagesByRegistrationSequence						m_imagesbyRegistrationSequence;

		void GetPalette(void);
		static void LoadWal(std::string fileName, byte **pic, unsigned int &width, unsigned int &height);
		static void LoadPCX(byte* raw, int len, byte **pic, byte **palette, unsigned int &width, unsigned int &height);

		void						UploadScratchImage(DirectX::ScratchImage & image, ID3D11Resource** pResource, bool generateMipMap);

		bool						UploadTexture2D(Texture2D *texture);
	public:
		bool						Initialize();
		void						Shutdown();

		DirectX::PackedVector::XMCOLOR	m_8to32table[256];
		DirectX::PackedVector::XMCOLOR	m_rawPalette[256];

		std::shared_ptr<Texture2D>		m_conChars;
		std::shared_ptr<Texture2D>		m_rawTexture;

		void						SetRawPalette(const unsigned char *palette);

		std::shared_ptr<Texture2D>	Load(std::string name, imagetype_t type);

		std::shared_ptr<Texture2D>	CreateTexture2DFromRaw(std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, DirectX::PackedVector::XMCOLOR *palette, D3D11_USAGE usage);
	};

	
}

#endif // !__DX11_IMAGE_HPP__