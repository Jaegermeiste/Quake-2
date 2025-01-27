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

#ifndef __DX12_RESOURCE_HPP__
#define __DX12_RESOURCE_HPP__
#pragma once

#include "dx12_local.hpp"

using boost::multi_index_container;
using namespace boost::multi_index;

typedef struct resourceHandleQ2_s {
	dxhandle_t m_handle;
} resourceHandleQ2_t;

namespace dx12
{
	typedef enum resourceType_e {
		RESOURCE_NONE,
		RESOURCE_TEXTURE2D,
		RESOURCE_MAX
	} resourceType_t;

	class Resource : public std::enable_shared_from_this<Resource>
	{
		friend class ResourceManager;

	protected:
		dxhandle_t				m_handle;
		std::string				m_name;
		resourceType_t			m_type;
		D3D12_RESOURCE_DESC		m_resourceDesc;
		ID3D12Resource*			m_resource = nullptr;

	public:
		Resource(std::string name) {
			m_name = name; 
			m_type = RESOURCE_NONE;
			m_handle = std::hash<std::string>{}(name);
			memset(&m_resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));
			m_resource = nullptr;
		};

		const	dxhandle_t					GetHandle()		const	{ return m_handle; };
		const	std::string					GetName()		const	{ return m_name; };
		const	std::string_view			GetNameView()	const	{ return m_name; };
		const	resourceType_t				GetType()		const	{ return m_type; };

		void								UpdateDesc()			{ if (m_resource != nullptr) { m_resourceDesc = m_resource->GetDesc(); } };

		virtual ~Resource() {}
	};

	struct tag_handle {};
	struct tag_name {};
	struct tag_type {};

	typedef multi_index_container<
		std::shared_ptr<Resource>,
		indexed_by<
			random_access<>,
			ordered_unique<
				tag<tag_handle>, const_mem_fun< Resource, const dxhandle_t, &Resource::GetHandle > 
			>,
			ordered_unique<
				tag<tag_name>, const_mem_fun< Resource, const std::string_view, &Resource::GetNameView > 
			>,
			ordered_non_unique<
				tag<tag_type>, const_mem_fun< Resource, const resourceType_t, &Resource::GetType > 
			>
		>
	> ResourceSet;

	class ResourceManager		// Factory
	{
		friend class Resource;
	private:
		std::unique_ptr<ResourceUploadBatch>								m_uploader;
		ResourceSet															m_resources;
		std::unordered_map<dxhandle_t, std::shared_ptr<resourceHandleQ2_t>>	m_handlesQ2;

	protected:
		static dxhandle_t													GenerateHandleForString (std::string string);

	public:
		bool																Initialize();
		void																Shutdown();


		//std::shared_ptr<Resource>											GetResource				(dxhandle_t handle);
		//std::shared_ptr<Resource>											GetResource				(std::string name);
		template <DerivedFrom<dx12::Resource> T>
		std::shared_ptr<T>											        GetResource             (dxhandle_t handle);
		template <DerivedFrom<dx12::Resource> T>
		std::shared_ptr<T>											        GetResource             (std::string name);

		template <DerivedFrom<dx12::Resource> T>
		std::shared_ptr<T>											        GetOrCreateResource		(std::string name);
		
		template <DerivedFrom<dx12::Resource> T>
		std::shared_ptr<T> 											        CreateResource          (std::string name);


		resourceHandleQ2_t*													GetResourceHandleQuake2	(dxhandle_t handle, bool validate = false);
	};
}

#endif // !__DX12_RESOURCE_HPP__