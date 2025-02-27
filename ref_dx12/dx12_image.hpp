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

#ifndef __DX12_IMAGE_HPP__
#define __DX12_IMAGE_HPP__
#pragma once

#include "dx12_local.hpp"

constexpr auto BPP_8 = 8;
constexpr auto BPP_24 = 24;
constexpr auto BPP_32 = 32;

constexpr auto WHITE_TEXTURE_NAME = L"WHITE";
constexpr auto CHECKERBOARD_TEXTURE_NAME = L"CHECKERBOARD";

namespace dx12
{
	typedef struct dds_header_s
	{
		uint32_t        size;
		uint32_t        flags;
		uint32_t        height;
		uint32_t        width;
		uint32_t        pitchOrLinearSize;
		uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		uint32_t        mipMapCount;
		uint32_t        reserved1[11];
		uint32_t        ddspfSize;
		uint32_t        ddspfFlags;
		uint32_t        ddspfFourCC;
		uint32_t        ddspfRGBBitCount;
		uint32_t        ddspfRBitMask;
		uint32_t        ddspfGBitMask;
		uint32_t        ddspfBBitMask;
		uint32_t        ddspfABitMask;
		uint32_t        caps1;
		uint32_t        caps2;
		uint32_t        caps3;
		uint32_t        caps4;
		uint32_t        reserved2;
	} dds_header_t;

	class ImageManager
	{
		friend class Draw;
	private:
		std::vector<std::wstring> m_imageExtensions =
		{
			L"dds", L"exr", L"hdr", L"tga", L"png", L"jpg", L"tif", L"gif", L"bmp", L"ico", L"wdp", L"jxr", L"wal", L"pcx"
		};

		qhandle_t				m_lastHandle = 0;

		std::unique_ptr<ResourceUploadBatch>    m_resourceUploadBatch = nullptr;

		void GetPalette(void);
		static void LoadWAL(byte* raw, size_t bufferLength, byte **pic, size_t &width, size_t &height);
		static void LoadPCX(byte* raw, size_t bufferLength, byte **pic, byte **palette, size_t &width, size_t &height);
		static void LoadTGA(byte* raw, size_t bufferLength, byte** pic, UINT64& width, unsigned int& height);

		D3D12_SUBRESOURCE_DATA  	        DepalletizeRawData(size_t width, size_t height, unsigned int bpp, byte* raw, XMCOLOR* palette);
		std::vector<D3D12_SUBRESOURCE_DATA> GetSubresourcesFromScratch(ScratchImage& scratch);
		std::vector<D3D12_SUBRESOURCE_DATA> GetSubresourcesFromResource(ComPtr<ID3D12Resource> resource);

		bool                                UploadSubresources(std::shared_ptr<dx12::Texture> texture);

		byte*                               LoadImageFile(std::wstring name, imagetype_t type, std::wstring &fileFormat, size_t &bufferSize);

		void                                ClearSubresources(std::shared_ptr<Texture> texture, size_t size);
	
	public:
		bool				    			Initialize();
		void					    		Shutdown();

		XMCOLOR						    	m_8to32table[256] = {};
		XMCOLOR							    m_rawPalette[256] = {};

		std::shared_ptr<Texture>    		m_conChars = nullptr;
		std::shared_ptr<Texture>	    	m_rawTexture = nullptr;
		std::shared_ptr<Texture>		    m_whiteTexture = nullptr;
		std::shared_ptr<Texture>		    m_checkerboardTexture = nullptr;

		std::vector<std::shared_ptr<Texture>> m_textures;

		void						        SetRawPalette(const unsigned char *palette);

		

		std::shared_ptr<Texture>	        Load(std::wstring name, imagetype_t type);
		std::shared_ptr<Texture>	        LoadSky(std::wstring name);

		std::shared_ptr<Texture>            CreateTexture(std::wstring name, imagetype_t type, size_t width, size_t height, bool generateMipmaps, DXGI_FORMAT format, D3D12_RESOURCE_DIMENSION dimension, size_t arraySize);
		std::shared_ptr<Texture>            CreateTexture(std::wstring name, imagetype_t type, size_t width, size_t height, bool generateMipmaps, DXGI_FORMAT format, TEX_DIMENSION dimension, size_t arraySize);
		std::shared_ptr<Texture> 	        CreateTextureFromRaw(std::wstring name, size_t width, size_t height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette);
		void                     	        UpdateTexture(std::shared_ptr<Texture> texture);
		void                    	        UpdateTextureFromRaw(std::shared_ptr<Texture> texture, size_t width, size_t height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette);

		void                                Imagelist_f();
	};


}

#endif // !__DX12_IMAGE_HPP__