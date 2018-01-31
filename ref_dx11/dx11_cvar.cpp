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
	LOG_FUNC();

	vid_ref					= std::make_shared<Cvar>("vid_ref",							"dx11",														CVAR_ARCHIVE);
	vid_xPos				= std::make_shared<Cvar>("vid_xpos",						0,															0);
	vid_yPos				= std::make_shared<Cvar>("vid_ypos",						0,															0);
	vid_fullscreen			= std::make_shared<Cvar>("vid_fullscreen",					0,															CVAR_ARCHIVE);
	vid_gamma				= std::make_shared<Cvar>("vid_gamma",						1,															CVAR_ARCHIVE);

	r_mode					= std::make_shared<Cvar>("r_mode",							-1,															CVAR_ARCHIVE);
#ifndef _DEBUG
	r_customWidth			= std::make_shared<Cvar>("r_customwidth",					1920,														CVAR_ARCHIVE);
	r_customHeight			= std::make_shared<Cvar>("r_customheight",					1080,														CVAR_ARCHIVE);
#else
	r_customWidth			= std::make_shared<Cvar>("r_customwidth",					1600,														CVAR_ARCHIVE);
	r_customHeight			= std::make_shared<Cvar>("r_customheight",					900,														CVAR_ARCHIVE);
#endif

	featureLevel			= std::make_shared<Cvar>("dx11_featureLevel",				"D3D_FEATURE_LEVEL_12_1",									CVAR_ARCHIVE);	// Leave this on 12_1, even on dx11
	bufferCount				= std::make_shared<Cvar>("dx11_bufferCount",				2,															CVAR_ARCHIVE);
	backBufferFormat		= std::make_shared<Cvar>("dx11_backBufferFormat",			"DXGI_FORMAT_R8G8B8A8_UNORM",								CVAR_ARCHIVE);
	Vsync					= std::make_shared<Cvar>("dx11_Vsync",						1,															CVAR_ARCHIVE);
	samplesPerPixel			= std::make_shared<Cvar>("dx11_samplesPerPixel",			2,															CVAR_ARCHIVE);	// 1 disables multisampling

	zNear2D					= std::make_shared<Cvar>("dx11_zNear2D",					0,															CVAR_ARCHIVE);
	zNear3D					= std::make_shared<Cvar>("dx11_zNear3D",					4,															CVAR_ARCHIVE);
	zFar2D					= std::make_shared<Cvar>("dx11_zFar2D",						99999,														CVAR_ARCHIVE);
	zFar3D					= std::make_shared<Cvar>("dx11_zFar3D",						4096,														CVAR_ARCHIVE);

#ifndef _DEBUG
	overlayScale			= std::make_shared<Cvar>("dx11_overlayScale",				1.0,														CVAR_ARCHIVE);
#else
	overlayScale			= std::make_shared<Cvar>("dx11_overlayScale",				0.9,														CVAR_ARCHIVE);
#endif
	overlayBrightness		= std::make_shared<Cvar>("dx11_overlayBrightness",			0.1125,														CVAR_ARCHIVE);
	overlayContrast			= std::make_shared<Cvar>("dx11_overlayContrast",			1.05,														CVAR_ARCHIVE);

	// Courtesy Kirk Barnes @ http://quake2xp.sourceforge.net/
	xpLitDownloadEnable		= std::make_shared<Cvar>("dx11_xpLitDownloadEnable",		false,														CVAR_ARCHIVE);
	xpLitDownloadPath		= std::make_shared<Cvar>("dx11_xpLitDownloadPath",			"https://sourceforge.net/p/quake2xp/code/HEAD/tree/",		CVAR_ARCHIVE);
	xpLitPathBaseQ2			= std::make_shared<Cvar>("dx11_xpLitPathBaseQ2",			"maps/",													CVAR_ARCHIVE);
	xpLitPathRogue			= std::make_shared<Cvar>("dx11_xpLitPathRogue",				"mapsr/",													CVAR_ARCHIVE);
	xpLitPathXatrix			= std::make_shared<Cvar>("dx11_xpLitPathXatrix",			"mapsx/",													CVAR_ARCHIVE);

	font					= std::make_shared<Cvar>("dx11_font",						"Courier New",												CVAR_ARCHIVE);
}

dx11::Cvars::Cvar::Cvar(std::string name, std::string defaultString, unsigned int flags)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	LOG(info) << "<name> " << name << " <string> " << defaultString << " <flags> " << CvarFlagsToString(flags);

	m_clientMemPtr = std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(defaultString.c_str()), msl::utilities::SafeInt<int>(flags)));
}

dx11::Cvars::Cvar::Cvar(std::string name, float defaultValue, unsigned int flags)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

	m_clientMemPtr = std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
}

dx11::Cvars::Cvar::Cvar(std::string name, double defaultValue, unsigned int flags)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

	m_clientMemPtr = std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
}

dx11::Cvars::Cvar::Cvar(std::string name, int defaultValue, unsigned int flags)
{
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

	m_clientMemPtr = std::make_unique<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
}

float dx11::Cvars::Cvar::Float()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return  m_clientMemPtr->value;
}

bool dx11::Cvars::Cvar::Bool()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	if (m_clientMemPtr->value > 0.0)
	{
		return true;
	}
	return false;
}

int dx11::Cvars::Cvar::Int()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return msl::utilities::SafeInt<int>(static_cast<int>(round(m_clientMemPtr->value)));
}

unsigned int dx11::Cvars::Cvar::UInt()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return msl::utilities::SafeInt<unsigned int>(static_cast<unsigned int>(round(m_clientMemPtr->value)));
}

double dx11::Cvars::Cvar::Double()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return  m_clientMemPtr->value;
}

std::string dx11::Cvars::Cvar::String()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return  std::string(m_clientMemPtr->string);
}

std::string dx11::Cvars::Cvar::LatchedString()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return  std::string(m_clientMemPtr->latched_string);
}

std::string dx11::Cvars::Cvar::Name()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return  std::string(m_clientMemPtr->name);
}

unsigned int dx11::Cvars::Cvar::Flags()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	return msl::utilities::SafeInt<unsigned int>(m_clientMemPtr->flags);
}

bool dx11::Cvars::Cvar::Modified()
{
	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	if (m_clientMemPtr->modified)
	{
		return true;
	}
	return false;
}

inline std::string dx11::Cvars::Cvar::CvarFlagsToString(unsigned flags)
{
	std::string flagString = "";

	if (flags & CVAR_LATCH)
	{
		flagString += "CVAR_LATCH ";
	}

	if (flags & CVAR_NOSET)
	{
		flagString += "CVAR_NOSET ";
	}

	if (flags & CVAR_SERVERINFO)
	{
		flagString += "CVAR_SERVERINFO ";
	}

	if (flags & CVAR_USERINFO)
	{
		flagString += "CVAR_USERINFO ";
	}

	if (flags & CVAR_ARCHIVE)
	{
		flagString += "CVAR_ARCHIVE";
	}

	if (flagString == "")
	{
		flagString = "__no_flags__";
	}

	return flagString;
}

bool dx11::Cvars::Cvar::InfoValidate(std::string value)
{
	LOG_FUNC();

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
	LOG_FUNC();

	// Wait for exclusive access
	std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

	// Cvar_Set is costly and poorly implemented when we already have a pointer to the cvar (linear string search)
	// so avoid calling it if at all possible
	if (value.compare(m_clientMemPtr->string) == 0)
	{
		return;		// not changed
	}

	if (m_clientMemPtr->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if (!InfoValidate(value))
		{
			ref->client->Con_Printf(PRINT_ALL, "invalid info cvar value\n");
			return;
		}
	}

	if (m_clientMemPtr->flags & CVAR_NOSET)
	{
		ref->client->Con_Printf(PRINT_ALL, std::string(m_clientMemPtr->name) + " is write protected.\n");
		return;
	}

	if (m_clientMemPtr->flags & CVAR_LATCH)
	{
		if ((m_clientMemPtr->latched_string != nullptr) || (m_clientMemPtr->latched_string != NULL))
		{
			if (value.compare(m_clientMemPtr->latched_string) == 0)
			{
				return;
			}
		}
	}

	// Cvar_Set uses some client side functionality for latching, server state, etc that we can't safely replicate here, so we have to use it
	ref->client->Cvar_Set(m_clientMemPtr->name, const_cast<char*>(value.c_str()));
}

void dx11::Cvars::Cvar::Set(float value)
{
	Set(std::to_string(value));
}

void dx11::Cvars::Cvar::Set(double value)
{
	Set(std::to_string(value));
}

void dx11::Cvars::Cvar::SetModified(bool value)
{
	LOG_FUNC();

	if (value == Modified())
	{
		return;		// not changed
	}
	else
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		if (value == true)
		{
			m_clientMemPtr->modified = true;
		}
		else
		{
			m_clientMemPtr->modified = false;
		}
	}
}

void dx11::Cvars::Cvar::Set(bool value)
{
	Set(std::to_string(value));
}

void dx11::Cvars::Cvar::Set(signed int value)
{
	Set(std::to_string(value));
}

void dx11::Cvars::Cvar::Set(unsigned int value)
{
	Set(std::to_string(value));
}