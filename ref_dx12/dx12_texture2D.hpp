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

	private:
		dxhandle_t               	m_srvHandle = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cachedD3D12SRVHandle = {};

		void                        RefreshSRV();

	public:
		imagetype_t					m_imageType;
		std::wstring				m_format;

		unsigned int				m_registrationSequence = 0;

		Texture2D(std::wstring name) : Resource(name) 
		{ 
			m_type      = RESOURCE_TEXTURE2D;
			m_imageType = it_pic;
			m_format    = L"UNKNOWN";
			m_srvHandle = {};
		};

		unsigned int				GetWidth() { return m_resourceDesc.Width; };
		unsigned int				GetHeight() { return m_resourceDesc.Height; };

		void                        CreateSRV();
		void                        BindSRV(std::shared_ptr<CommandList> commandList);
	};
}

template std::shared_ptr<dx12::Texture2D> dx12::ResourceManager::CreateResource<dx12::Texture2D>(std::wstring name);
template std::shared_ptr<dx12::Texture2D> dx12::ResourceManager::GetOrCreateResource<dx12::Texture2D>(std::wstring name);
template std::shared_ptr<dx12::Texture2D> dx12::ResourceManager::GetResource<dx12::Texture2D>(std::wstring name);
template std::shared_ptr<dx12::Texture2D> dx12::ResourceManager::GetResource<dx12::Texture2D>(dxhandle_t handle);

#endif//__DX12_TEXTURE2D_HPP__