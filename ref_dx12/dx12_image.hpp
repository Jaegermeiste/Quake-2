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
	class ImageManager
	{
		friend class Draw;
	private:
		std::vector<std::wstring> m_imageExtensions =
		{
			L"dds", L"exr", L"hdr", L"tga", L"png", L"jpg", L"tif", L"gif", L"bmp", L"ico", L"wdp", L"jxr", L"wal", L"pcx"
		};

		qhandle_t				m_lastHandle = 0;

		void GetPalette(void);
		static void LoadWal(std::wstring fileName, byte **pic, UINT64 &width, unsigned int &height);
		static void LoadPCX(byte* raw, int len, byte **pic, byte **palette, UINT64 &width, unsigned int &height);

		
	
	public:
		bool							Initialize();
		void							Shutdown();

		XMCOLOR							m_8to32table[256] = {};
		XMCOLOR							m_rawPalette[256] = {};

		std::shared_ptr<Texture2D>		m_conChars = nullptr;
		std::shared_ptr<Texture2D>		m_rawTexture = nullptr;
		std::shared_ptr<Texture2D>		m_whiteTexture = nullptr;
		std::shared_ptr<Texture2D>		m_checkerboardTexture = nullptr;

		void						    SetRawPalette(const unsigned char *palette);

		std::shared_ptr<Texture2D>	    Load(std::wstring name, imagetype_t type);

		std::shared_ptr<Texture2D> 	    CreateTexture2DFromRaw(std::wstring name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette);
		void                     	    UpdateTexture2DFromRaw(std::shared_ptr<Texture2D> texture, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette, bool update = true, std::shared_ptr<CommandList> commandList = nullptr);
	};


}

#endif // !__DX12_IMAGE_HPP__