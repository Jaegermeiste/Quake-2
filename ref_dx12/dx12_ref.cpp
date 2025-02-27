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

namespace dx12
{
	std::unique_ptr<Ref> ref = nullptr;
}

void	dx12::Ref::Init(refimport_t rimp)
{
	LOG_FUNC();

	try
	{
		client = std::make_unique<Client>(rimp);
		cvars = std::make_unique<Cvars>();
		res = std::make_unique<ResourceManager>();
		media = std::make_unique<Media>();
		draw = std::make_unique<Draw>();
		sys = std::make_unique<System>();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}