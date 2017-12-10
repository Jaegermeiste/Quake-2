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
	public:
					Client					(refimport_t rimp);
					~Client					();

		void		SetRefImport			(refimport_t rimp);

		void		Sys_Error				(unsigned short err_level, std::string str);

		void		(*Cmd_AddCommand)		(char *name, void(*cmd)(void));
		void		(*Cmd_RemoveCommand)	(char *name);
		int			(*Cmd_Argc)				(void);
		char		*(*Cmd_Argv)			(int i);
		void		(*Cmd_ExecuteText)		(int exec_when, char *text);

		void		Con_Printf				(unsigned short print_level, std::string str);

		int			(*FS_LoadFile)			(char *name, void **buf);
		void		(*FS_FreeFile)			(void *buf);

		char		*(*FS_Gamedir)			(void);

		cvar_t		*(*Cvar_Get)			(char *name, char *value, int flags);
		cvar_t		*(*Cvar_Set)			(char *name, char *value);
		void		(*Cvar_SetValue)		(char *name, float value);

		qboolean	(*Vid_GetModeInfo)		(int *width, int *height, int mode);
		void		(*Vid_MenuInit)			(void);
		void		(*Vid_NewWindow)		(int width, int height);
	};

	Client* client = NULL;
}

#endif // !__DX12_CLIENT_HPP__