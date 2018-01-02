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

#include "dx11_local.hpp"

dx11::Cvars::Cvars()
{
	forceScreenRes		= std::make_shared<Cvar>("dx11_forceScreenRes",		1,								CVAR_ARCHIVE);
	featureLevel		= std::make_shared<Cvar>("dx11_featureLevel",		"D3D_FEATURE_LEVEL_11_1",		CVAR_ARCHIVE);
	bufferCount			= std::make_shared<Cvar>("dx11_bufferCount",		2,								CVAR_ARCHIVE);
	backBufferFormat	= std::make_shared<Cvar>("dx11_backBufferFormat",	"DXGI_FORMAT_R8G8B8A8_UNORM",	CVAR_ARCHIVE);
	Vsync				= std::make_shared<Cvar>("dx11_Vsync",				1,								CVAR_ARCHIVE);
	samplesPerPixel		= std::make_shared<Cvar>("dx11_samplesPerPixel",	2,								CVAR_ARCHIVE);	// 1 disables multisampling
}

dx11::Cvars::Cvar::Cvar(std::string name, std::string defaultString, unsigned int flags)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	std::atomic_store(&atomicSharedPtr, std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(defaultString.c_str()), flags)));
}

dx11::Cvars::Cvar::Cvar(std::string name, float defaultValue, unsigned int flags)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	std::atomic_store(&atomicSharedPtr, std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), flags)));
}

float dx11::Cvars::Cvar::Float()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return  std::atomic_load(&atomicSharedPtr)->value;
}

bool dx11::Cvars::Cvar::Bool()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	if (std::atomic_load(&atomicSharedPtr)->value > 0.0)
	{
		return true;
	}
	return false;
}

int dx11::Cvars::Cvar::Int()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return msl::utilities::SafeInt<int>(round(std::atomic_load(&atomicSharedPtr)->value));
}

unsigned int dx11::Cvars::Cvar::UInt()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return msl::utilities::SafeInt<unsigned int>(round(std::atomic_load(&atomicSharedPtr)->value));
}

double dx11::Cvars::Cvar::Double()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return  std::atomic_load(&atomicSharedPtr)->value;
}

std::string dx11::Cvars::Cvar::String()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return  std::string(std::atomic_load(&atomicSharedPtr)->string);
}

std::string dx11::Cvars::Cvar::LatchedString()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return  std::string(std::atomic_load(&atomicSharedPtr)->latched_string);
}

std::string dx11::Cvars::Cvar::Name()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return  std::string(std::atomic_load(&atomicSharedPtr)->name);
}

unsigned int dx11::Cvars::Cvar::Flags()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	return msl::utilities::SafeInt<unsigned int>(std::atomic_load(&atomicSharedPtr)->flags);
}

bool dx11::Cvars::Cvar::Modified()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	if (std::atomic_load(&atomicSharedPtr)->modified > 0)
	{
		return true;
	}
	return false;
}

bool dx11::Cvars::Cvar::InfoValidate(std::string value)
{
	std::string::size_type found = value.find("\\");

	if (found != std::string::npos)
	{
		return false;
	}

	found = value.find("\"");
	if (found != std::string::npos)
	{
		return false;
	}

	found = value.find(";");
	if (found != std::string::npos)
	{
		return false;
	}

	return true;
}


void dx11::Cvars::Cvar::Set(std::string value)
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(ptrAccessMutex);

	// Cvar_Set is costly and poorly implemented when we already have a pointer to the cvar (linear string search)
	// so avoid calling it if at all possible
	if (value.compare(std::atomic_load(&atomicSharedPtr)->string) == 0)
	{
		return;		// not changed
	}

	if (std::atomic_load(&atomicSharedPtr)->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!InfoValidate(value))
		{
			ref->client->Con_Printf(PRINT_ALL, "invalid info cvar value\n");
			return;
		}
	}

	if (std::atomic_load(&atomicSharedPtr)->flags & CVAR_NOSET)
	{
		ref->client->Con_Printf(PRINT_ALL, std::string(std::atomic_load(&atomicSharedPtr)->name) + " is write protected.\n");
		return;
	}

	if (std::atomic_load(&atomicSharedPtr)->flags & CVAR_LATCH)
	{
		if ((std::atomic_load(&atomicSharedPtr)->latched_string != nullptr) || (std::atomic_load(&atomicSharedPtr)->latched_string != NULL))
		{
			if (value.compare(std::atomic_load(&atomicSharedPtr)->latched_string) == 0)
			{
				return;
			}
		}
	}

	// Cvar_Set uses some client side functionality for latching, server state, etc that we can't safely replicate here, so we have to use it
	ref->client->Cvar_Set(std::atomic_load(&atomicSharedPtr)->name, const_cast<char*>(value.c_str()));
}

inline void dx11::Cvars::Cvar::Set(float value)
{
	Set(std::to_string(value));
}

inline void dx11::Cvars::Cvar::Set(double value)
{
	Set(std::to_string(value));
}

inline void dx11::Cvars::Cvar::Set(bool value)
{
	Set(std::to_string(value));
}

inline void dx11::Cvars::Cvar::Set(signed int value)
{
	Set(std::to_string(value));
}

inline void dx11::Cvars::Cvar::Set(unsigned int value)
{
	Set(std::to_string(value));
}