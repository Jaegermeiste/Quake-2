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
2017 Bleeding Eye Studios
*/

#ifndef __DX12_REF_HPP__
#define __DX12_REF_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Ref {
	private:

	public:
		void					Init(refimport_t rimp);

		std::unique_ptr<Cvars>				cvars;
		std::unique_ptr<Client>				client;
		std::unique_ptr<ResourceManager>	res;
		std::unique_ptr<Media>				media;
		std::unique_ptr<Draw>				draw;
		std::unique_ptr<System>				sys;
	};

	extern std::unique_ptr<Ref> ref;
}

#endif // !__DX12_REF_HPP__