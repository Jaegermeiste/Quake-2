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

typedef struct resourceHandleQ2_s {
	handle_t m_handle;
} resourceHandleQ2_t;

typedef resourceHandleQ2_t image_t;
typedef resourceHandleQ2_t model_t;

namespace dx12
{
	typedef enum resourceType_e {
		RESOURCE_NONE,
		RESOURCE_TEXTURE2D,
		RESOURCE_MAX
	} resourceType_t;

	class Resource : public std::enable_shared_from_this<Resource>
	{
	private:
		std::string		m_name;
		handle_t		m_handle;
		resourceType_t	m_type;

	public:
		Resource(std::string name, resourceType_t type) { m_name = name; m_type = type; };

		const	handle_t					GetHandle() { return m_handle; };
	};

	class ResourceManager		// Factory
	{
	private:
		std::unordered_map<handle_t, std::shared_ptr<Resource>>				resources;
		std::unordered_map<handle_t, std::shared_ptr<resourceHandleQ2_t>>	flyweights;

		handle_t															GenerateHandleForString	(std::string string);
		handle_t															GenerateHandleForStringAndType(std::string string, resourceType_t type);

		std::shared_ptr<Resource>											CreateResource			(std::string name, resourceType_t type);

	public:
		bool																Initialize();
		void																Shutdown();


		std::shared_ptr<Resource>											GetResource				(handle_t handle);
		std::shared_ptr<Resource>											GetResource				(std::string name, resourceType_t type);
		std::shared_ptr<Resource>											GetOrCreateResource		(std::string name, resourceType_t type);


		resourceHandleQ2_t*													GetResourceHandleQuake2	(handle_t handle, bool validate = false);
	};
}

#endif // !__DX12_RESOURCE_HPP__