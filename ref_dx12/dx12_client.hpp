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

#ifndef __DX12_CLIENT_HPP__
#define __DX12_CLIENT_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Client {
	private:
		refimport_t	ri;

		std::mutex	refImportMutex;

	public:
						Client					(refimport_t rimp);
						~Client					();

		void			SetRefImport			(refimport_t rimp);

		inline	void			Sys_Error				(unsigned short err_level, std::string str);

		void			(*Cmd_AddCommand)		(char *name, void(*cmd)(void));
		inline	void			Cmd_RemoveCommand		(std::string name);
		inline	unsigned int	Cmd_Argc				(void);
		char			*(*Cmd_Argv)			(int i);
		void			(*Cmd_ExecuteText)		(int exec_when, char *text);

		inline	void			Con_Printf				(unsigned short print_level, std::string str);

		int				(*FS_LoadFile)			(char *name, void **buf);
		void			(*FS_FreeFile)			(void *buf);

		inline	std::string		FS_Gamedir			(void);

		cvar_t			*(*Cvar_Get)			(char *name, char *value, int flags);
		cvar_t			*(*Cvar_Set)			(char *name, char *value);
		void			(*Cvar_SetValue)		(char *name, float value);

		inline	bool			Vid_GetModeInfo			(unsigned int &width, unsigned int &height, int mode);
		inline	void			Vid_MenuInit			(void);
		inline	void			Vid_NewWindow			(unsigned int width, unsigned int height);
	};
}

#endif // !__DX12_CLIENT_HPP__