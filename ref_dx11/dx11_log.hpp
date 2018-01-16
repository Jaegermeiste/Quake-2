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
ref_dx11
2017 Bleeding Eye Studios
*/

#ifndef __DX11_LOG_HPP__
#define __DX11_LOG_HPP__
#pragma once

#include "dx11_local.hpp"

#define LOG_FILE_NAME	"%y%m%d_%H%M_ref_dx11.log"
#define LOG_PATH		"logs/ref_dx11"

#define LOG(level) BOOST_LOG_TRIVIAL(level)

enum severity_level
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

namespace dx11
{
	class Log 
	{
	public:
		Log();
	};

	extern std::unique_ptr<Log> log;
}

#endif // !__DX11_LOG_HPP__