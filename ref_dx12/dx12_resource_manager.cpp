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

inline dxhandle_t dx12::ResourceManager::GenerateHandleForString(std::wstring string)
{
	try
	{
		// Resources should never have the same name and path, so hashes should (almost) never collide
		return std::hash<std::wstring>{}(string);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

bool dx12::ResourceManager::Initialize()
{
	LOG_FUNC();

	return true;
}

void dx12::ResourceManager::Shutdown()
{
	LOG_FUNC();

	try
	{
		// Destroy m_handlesQ2
		m_handlesQ2.clear();

		// Destroy resources
		m_resources.clear();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

/*std::shared_ptr<dx12::Resource> dx12::ResourceManager::GetResource(dxhandle_t handle)
{
	LOG_FUNC();

	try {
		if (!m_resources.empty()) {
			auto result = m_resources.get<tag_handle>().find(handle);

			if (result != m_resources.get<tag_handle>().end())
			{
				// Found the resource
				return std::make_shared<dx12::Resource>(*result);
			}
			else
			{
				LOG(warning) << "Resource for handle " << handle << " not found.";
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}*/

template<DerivedFrom<dx12::Resource> T>
std::shared_ptr<T> dx12::ResourceManager::GetResource(dxhandle_t handle)
{
	LOG_FUNC();

	try {
		if (!m_resources.empty()) {
			auto result = m_resources.get<tag_handle>().find(handle);

			if (result != m_resources.get<tag_handle>().end())
			{
				std::shared_ptr<Resource> resource = *result;
				auto dummy = std::make_shared<T>(L"dummy");

				// Found the resource, see if it is the right type
				if (resource->GetType() == dummy->GetType()) {
					return std::dynamic_pointer_cast<T>(resource);
				}

				LOG(warning) << "Resource for handle " << handle << " found, but wrong type.";

				return std::dynamic_pointer_cast<T>(resource);
			}
			else
			{
				LOG(warning) << "Resource for handle " << handle << " not found.";
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

/*std::shared_ptr<dx12::Resource> dx12::ResourceManager::GetResource(std::string name)
{
	LOG_FUNC();

	dxhandle_t handle = GenerateHandleForString(name);

	return GetResource(handle);
}*/

template<DerivedFrom<dx12::Resource> T>
std::shared_ptr<T> dx12::ResourceManager::GetResource(std::wstring name)
{
	LOG_FUNC();

	try
	{
		dxhandle_t handle = GenerateHandleForString(name);

		return GetResource<T>(handle);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

template<DerivedFrom<dx12::Resource> T>
std::shared_ptr<T> dx12::ResourceManager::GetOrCreateResource(std::wstring name)
{
	LOG_FUNC();

	try
	{
		std::shared_ptr<T> resource = nullptr;

		resource = GetResource<T>(name);

		if (resource == nullptr)
		{
			// Resource not found
			resource = CreateResource<T>(name);
		}

		return resource;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

resourceHandleQ2_t* dx12::ResourceManager::GetResourceHandleQuake2(dxhandle_t handle)
{
	LOG_FUNC();

	resourceHandleQ2_t* q2handle = nullptr;

	try
	{
		// Check if handle is a valid resource
		auto resource = GetResource<Resource>(handle);
		if (resource)
		{
			if (resource->GetHandle() != handle)
			{
				// Failed validation
				LOG(error) << "Provided handle " << handle << " does not match resource handle " << resource->GetHandle();

				return nullptr;
			}

			// At this point, assume the handle is good and create the flyweight struct
			m_handlesQ2.insert({ handle, std::make_shared<resourceHandleQ2_t>() });

			// Set the handle
			m_handlesQ2.at(handle)->m_handle = handle;

			// Name it, because the client does evil things to the handle for skins
			sz::memcpy(m_handlesQ2.at(handle)->m_name, ref->sys->ToString(resource->GetName()).c_str(), MAX_QPATH);

			// Retrieve a raw pointer to the flyweight
			q2handle = m_handlesQ2.at(handle).get();
		}
		else
		{
			// Failed validation
			LOG(error) << "Resource for handle " << handle << " not found.";

			return nullptr;
		}

		
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return q2handle;
}

template<DerivedFrom<dx12::Resource> T>
std::shared_ptr<T> dx12::ResourceManager::CreateResource(std::wstring name)
{
	LOG_FUNC();

	try
	{
		if (GetResource<T>(name) == nullptr)
		{
			// Resource not found already
			auto result = m_resources.push_back(std::make_shared<T>(name));

			if (result.second == true)
			{
				// Insertion succeeded. Return the pointer to the resource
				return std::dynamic_pointer_cast<T>(*result.first);
			}
			else
			{
				LOG(warning) << "Resource insertion blocked by existing resource handle: " << std::to_wstring((*result.first)->GetHandle()) << ", name: " << (*result.first)->GetName() << ".";
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}
