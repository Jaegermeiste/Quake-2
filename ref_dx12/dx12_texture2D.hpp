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

#ifndef __DX12_TEXTURE2D_HPP__
#define __DX12_TEXTURE2D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Texture2D : public Resource
	{
		friend class ResourceManager;
		friend class ImageManager;
	public:
		D3D12_RESOURCE_DESC		m_textureDesc;
		//ID3D12ShaderResourceView*	m_shaderResourceView = nullptr;
		ID3D12Resource*				m_resource = nullptr;
		ID3D12Resource*			m_texture2D = nullptr;
		imagetype_t					m_imageType;
		std::string					m_format;

		qhandle_t					m_handle;
		std::string					m_name;
		unsigned int				m_registrationSequence = 0;

		Texture2D(std::string name) : Resource(name, RESOURCE_TEXTURE2D) {};
	};
}

#endif//__DX12_TEXTURE2D_HPP__