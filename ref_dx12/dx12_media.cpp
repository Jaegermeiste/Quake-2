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

dx12::Media::Media()
{
	m_inRegistration = false;

	img = std::make_unique<ImageManager>();
	model = std::make_unique<ModelManager>();
	map = std::make_unique<Map>();
}

bool dx12::Media::Initialize()
{
	if ((!img) || (!img->Initialize()))
	{
		LOG(error) << "Failed to create image subsystem.";
		return false;
	}

	if ((!model) || (!model->Initialize()))
	{
		LOG(error) << "Failed to create model subsystem.";
		return false;
	}

	if ((!map) || (!map->Initialize()))
	{
		LOG(error) << "Failed to create map subsystem.";
		return false;
	}

	return true;
}

void dx12::Media::Shutdown()
{
	LOG_FUNC();

	if (map)
	{
		map->Shutdown();
	}

	if (model)
	{
		model->Shutdown();
	}

	if (img)
	{
		img->Shutdown();
	}
}

void dx12::Media::BeginRegistration(std::string mapname)
{
	LOG_FUNC();

	if (!m_inRegistration)
	{
		m_inRegistration = true;
	}
}

void dx12::Media::EndRegistration()
{
	LOG_FUNC();

	if (m_inRegistration)
	{
		m_inRegistration = false;
	}
}