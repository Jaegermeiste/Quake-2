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

dx12::ModelManager::ModelManager()
{
	LOG_FUNC();
}

bool dx12::ModelManager::Initialize()
{
	LOG_FUNC();

	return true;
}

void dx12::ModelManager::Shutdown()
{
	LOG_FUNC();
}

std::shared_ptr<dx12::Resource> dx12::ModelManager::Load(std::string name)
{
	LOG_FUNC();

	return nullptr;
}
