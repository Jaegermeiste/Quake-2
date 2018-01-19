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

#ifndef __DX11_CLIENT_HPP__
#define __DX11_CLIENT_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class Client {
	friend class Cvar;
	private:
		refimport_t	m_refImport;

		std::mutex	m_refImportMutex;

		cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
		cvar_t			*(*Cvar_Set)			(char *name, char *value);
		void			(*Cvar_SetValue)		(char *name, float value);

		// Friends
		friend dx11::Cvars::Cvar::Cvar(std::string name, std::string defaultString, unsigned int flags);
		friend dx11::Cvars::Cvar::Cvar(std::string name, float defaultValue, unsigned int flags);
		friend dx11::Cvars::Cvar::Cvar(std::string name, double defaultValue, unsigned int flags);
		friend dx11::Cvars::Cvar::Cvar(std::string name, int defaultValue, unsigned int flags);
		friend void dx11::Cvars::Cvar::Set(std::string value);

	public:
						Client					(refimport_t rimp);
						~Client					();

		void			SetRefImport			(refimport_t rimp);

		void			Sys_Error				(unsigned short err_level, std::string str);

		void			Cmd_AddCommand			(std::string name, void(*cmd)());
		void			Cmd_RemoveCommand		(std::string name);
		unsigned int	Cmd_Argc				(void);
		std::string		Cmd_Argv				(unsigned int i);
		void			Cmd_ExecuteText			(unsigned int exec_when, std::string text);

		void			Con_Printf				(unsigned short print_level, std::string str);

		int				FS_LoadFile				(std::string fileName, void **buf);
		void			FS_FreeFile				(void *buf);

		std::string		FS_Gamedir				(void);
		std::string		FS_GamedirAbsolute		(void);		// Added to faciliate shader compiling

		bool			Vid_GetModeInfo			(unsigned int &width, unsigned int &height, int mode);
		void			Vid_MenuInit			(void);
		void			Vid_NewWindow			(unsigned int width, unsigned int height);
	};
}

#endif // !__DX11_CLIENT_HPP__