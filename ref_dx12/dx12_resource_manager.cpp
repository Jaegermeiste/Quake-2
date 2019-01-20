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

#include "dx12_local.hpp"

handle_t dx12::ResourceManager::GenerateHandleForString(std::string string)
{
	handle_t handle = std::hash<std::string>{}(string);

	return handle;
}

std::shared_ptr<dx12::Resource> dx12::ResourceManager::CreateResource(std::string name, resourceType_t type)
{
	LOG_FUNC();

	std::shared_ptr<Resource> resource = nullptr;
	handle_t handle = GenerateHandleForString(name);

	// At this point, assume the handle is good and create the asset
	resources.insert({ handle, std::make_shared<Resource>(name, handle) });

	// Retrieve a pointer to the flyweight
	resource = resources.at(handle);

	return resource;
}

void dx12::ResourceManager::Shutdown()
{
	LOG_FUNC();

	// Destroy flyweights
	flyweights.clear();

	// Destroy resources
	resources.clear();
}

std::shared_ptr<dx12::Resource> dx12::ResourceManager::GetResource(handle_t handle)
{
	LOG_FUNC();

	std::shared_ptr<Resource> resource = nullptr;

	if (resources.count(handle) > 0)
	{
		// Found the resource
		resource = resources.at(handle);
	}
	else
	{
		LOG(warning) << "Resource for handle " << handle << " not found.";
	}

	return resource;
}

std::shared_ptr<dx12::Resource> dx12::ResourceManager::GetResource(std::string name, resourceType_t type)
{
	LOG_FUNC();

	handle_t handle = GenerateHandleForStringAndType(name, type);

	return GetResource(handle);
}

std::shared_ptr<dx12::Resource> dx12::ResourceManager::GetOrCreateResource(std::string name, resourceType_t type)
{
	LOG_FUNC();

	std::shared_ptr<Resource> resource = nullptr;

	resource = GetResource(name, type);

	if (resource == nullptr)
	{
		// Resource not found
		resource = CreateResource(name, type);
	}

	return resource;
}

resourceHandleQ2_t* dx12::ResourceManager::GetResourceHandleQuake2(handle_t handle, bool validate)
{
	LOG_FUNC();

	resourceHandleQ2_t* fly = nullptr;

	if (validate)
	{
		// Check if handle is a valid resource
		auto resource = GetResource(handle);
		if (resource)
		{
			if (resource->GetHandle() != handle)
			{
				// Failed validation
				LOG(error) << "Provided handle " << handle << " does not match resource handle " << resource->GetHandle();

				return nullptr;
			}
		}
		else
		{
			// Failed validation
			LOG(error) << "Resource for handle " << handle << " not found.";

			return nullptr;
		}
	}
	
	// At this point, assume the handle is good and create the flyweight struct
	flyweights.insert({ handle, std::make_shared<resourceHandleQ2_t>()});

	// Set the handle
	flyweights.at(handle)->m_handle = handle;

	// Retrieve a raw pointer to the flyweight
	fly = flyweights.at(handle).get();

	return fly;
}