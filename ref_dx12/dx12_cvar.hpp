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

#ifndef __DX12_CVAR_HPP__
#define __DX12_CVAR_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Cvars {
	private:

	public:
		class Cvar {
			std::atomic<std::unique_ptr<cvar_t>> atomicSharedPtr;

			std::mutex ptrAccessMutex;

			static	bool			InfoValidate(std::string value);

		public:
									Cvar(std::string name, std::string defaultString, unsigned int flags);
									Cvar(std::string name, float defaultValue, unsigned int flags);

					bool			Bool();
					signed int		Int();
					unsigned int	UInt();
					float			Float();
					double			Double();
					std::string		String();
					std::string		LatchedString();
					std::string		Name();
					unsigned int	Flags();
					bool			Modified();

					void			Set(std::string string);
			inline	void			Set(bool value);
			inline	void			Set(signed int value);
			inline	void			Set(unsigned int value);
			inline	void			Set(float value);
			inline	void			Set(double value);
		};

		Cvars();
		~Cvars();

		std::shared_ptr<Cvar>  forceScreenRes;
		std::shared_ptr<Cvar>  featureLevel;
		std::shared_ptr<Cvar>  bufferCount;
		std::shared_ptr<Cvar>  backBufferFormat;
		std::shared_ptr<Cvar>  Vsync;
		std::shared_ptr<Cvar>  samplesPerPixel;
	};
};

#endif // !__DX12_CVAR_HPP__