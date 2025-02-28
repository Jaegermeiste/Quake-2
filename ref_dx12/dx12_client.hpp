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

#ifndef __DX12_CLIENT_HPP__
#define __DX12_CLIENT_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Client {
		friend class Cvar;
	private:
		refimport_t	m_refImport;

		std::mutex	m_refImportMutex;

		std::vector<char*>	m_cmdNames;

		cvar_t			*(*Cvar_Get)		(char *name, char *value, int flags);
		cvar_t			*(*Cvar_Set)		(char *name, char *value);
		void			(*Cvar_SetValue)	(char *name, float value);

		// Friends
		friend dx12::Cvars::Cvar::Cvar		(std::wstring name, std::wstring defaultString, unsigned int flags);
		friend dx12::Cvars::Cvar::Cvar		(std::wstring name, float defaultValue, unsigned int flags);
		friend dx12::Cvars::Cvar::Cvar		(std::wstring name, double defaultValue, unsigned int flags);
		friend dx12::Cvars::Cvar::Cvar		(std::wstring name, int defaultValue, unsigned int flags);
		friend void dx12::Cvars::Cvar::Set	(std::wstring value);

	public:
		Client(refimport_t rimp);
		~Client();

		void			SetRefImport		(refimport_t rimp);

		void			Cmd_AddCommand		(std::wstring name, void(*cmd)());
		void			Cmd_RemoveCommand	(std::wstring name);
		unsigned int	Cmd_Argc			(void);
		std::wstring	Cmd_Argv			(unsigned int i);
		void			Cmd_ExecuteText		(unsigned int exec_when, std::wstring text);

		template<typename T, typename... Args>
		void Con_Printf(unsigned short print_level, T formatString, Args&&... args)
		{
			LOG_FUNC();

			try
			{
				std::wstring printLevelStr = L"PRINT_ALL";

				if (print_level == PRINT_DEVELOPER)
				{
					printLevelStr = L"PRINT_DEVELOPER";
				}

				std::wstring outStr = L"";
				
				if (sizeof...(args) == 0)
				{
					outStr = ToWideString(formatString);
				}
				else
				{
					outStr = Format(formatString, std::forward<Args>(args)...);
				}

				// Wait for exclusive access
				std::lock_guard<std::mutex> guard(m_refImportMutex);

				LOG(info) << "[" << printLevelStr << "]: " << outStr;

				outStr += L"\n";

				m_refImport.Con_Printf(print_level, const_cast<char*>(ToString(outStr).c_str()));
			}
			catch (const std::runtime_error& e) {
				LOG(error) << "Runtime Error: " << e.what();
			}
			catch (const std::exception& e) {
				LOG(error) << "General Exception: " << e.what();
			}
		}

		template<typename T, typename... Args>
		void Sys_Error(unsigned short err_level, T formatString, Args&&... args)
		{
			LOG_FUNC();

			try
			{
				std::wstring errLevelStr = L"ERR_FATAL";

				if (err_level == ERR_DROP)
				{
					errLevelStr = L"ERR_DROP";
				}
				else if (err_level == ERR_QUIT)
				{
					errLevelStr = L"ERR_QUIT";
				}

				std::wstring outStr = L"";

				if (sizeof...(args) == 0)
				{
					outStr = ToWideString(formatString);
				}
				else
				{
					outStr = Format(formatString, std::forward<Args>(args)...);
				}

				// Wait for exclusive access
				std::lock_guard<std::mutex> guard(m_refImportMutex);

				LOG(info) << "[" << errLevelStr << "]: " << outStr;

				outStr += L"\n";

				m_refImport.Sys_Error(err_level, const_cast<char*>(ToString(outStr).c_str()));
			}
			catch (const std::runtime_error& e) {
				LOG(error) << "Runtime Error: " << e.what();
			}
			catch (const std::exception& e) {
				LOG(error) << "General Exception: " << e.what();
			}
		};

		int				FS_LoadFile			(std::wstring fileName, void **buf);
		void			FS_FreeFile			(void *buf);

		std::wstring	FS_Gamedir			(void);
		std::wstring	FS_GamedirAbsolute	(void);		// Added to faciliate shader compiling

		bool			Vid_GetModeInfo		(unsigned int &width, unsigned int &height, int mode);
		void			Vid_MenuInit		(void);
		void			Vid_NewWindow		(unsigned int width, unsigned int height);
	};
}

#endif // !__DX12_CLIENT_HPP__