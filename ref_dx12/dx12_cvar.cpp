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

dx12::Cvars::Cvars()
{
	LOG_FUNC();

	try
	{
		vid_ref					= std::make_shared<Cvar>(L"vid_ref", L"dx12",													CVAR_ARCHIVE);
		vid_xPos				= std::make_shared<Cvar>(L"vid_xpos",						0,														0);
		vid_yPos				= std::make_shared<Cvar>(L"vid_ypos",						0,														0);
		vid_fullscreen			= std::make_shared<Cvar>(L"vid_fullscreen",					0,														CVAR_ARCHIVE);
		vid_gamma				= std::make_shared<Cvar>(L"vid_gamma",						1,														CVAR_ARCHIVE);

		scr_drawall             = std::make_shared<Cvar>(L"scr_drawall",                  1,                                     CVAR_ARCHIVE);

		mode				    = std::make_shared<Cvar>(L"dx12_mode",						11,														CVAR_ARCHIVE);
	#if defined(DEBUG) || defined (_DEBUG)
		r_customWidth			= std::make_shared<Cvar>(L"r_customwidth",					1280,													CVAR_ARCHIVE);
		r_customHeight			= std::make_shared<Cvar>(L"r_customheight",					720,													CVAR_ARCHIVE);

		hdr10                   = std::make_shared<Cvar>(L"dx12_hdr10",                    1,                                        CVAR_ARCHIVE);
	#else
		r_customWidth			= std::make_shared<Cvar>(L"r_customwidth",					1920,													CVAR_ARCHIVE);
		r_customHeight			= std::make_shared<Cvar>(L"r_customheight",					1280,													CVAR_ARCHIVE);
	#endif
		flushmap				= std::make_shared<Cvar>(L"flushmap",						0,														0);

		// DX Specific
		featureLevel			= std::make_shared<Cvar>(L"dx12_featureLevel", L"D3D_FEATURE_LEVEL_12_2",								CVAR_ARCHIVE);	// Leave this on 12_1, even on dx12
		bufferCount				= std::make_shared<Cvar>(L"dx12_bufferCount",				2,														CVAR_ARCHIVE);
		backBufferFormat		= std::make_shared<Cvar>(L"dx12_backBufferFormat", L"DXGI_FORMAT_R8G8B8A8_UNORM",							CVAR_ARCHIVE);
		Vsync					= std::make_shared<Cvar>(L"dx12_Vsync",						1,														CVAR_ARCHIVE);
		samplesPerPixel			= std::make_shared<Cvar>(L"dx12_samplesPerPixel",			1,														CVAR_ARCHIVE);	// 1 disables multisampling

		shaderVertexSwap        = std::make_shared<Cvar>(L"dx12_shaderVertexSwap", L"swapChain.hlsl",                                     CVAR_ARCHIVE);
		shaderPixelSwap         = std::make_shared<Cvar>(L"dx12_shaderPixelSwap", L"swapChain.hlsl",                                      CVAR_ARCHIVE);

		shaderVertex2D          = std::make_shared<Cvar>(L"dx12_shaderVertex2D", L"subsystem2D.hlsl",                                     CVAR_ARCHIVE);
		shaderPixel2D           = std::make_shared<Cvar>(L"dx12_shaderPixel2D",  L"subsystem2D.hlsl",                                     CVAR_ARCHIVE);

		shaderRaygen3D          = std::make_shared<Cvar>(L"dx12_shaderRaygen3D", L"dxrRayGen.hlsl",                                       CVAR_ARCHIVE);
		shaderMiss3D            = std::make_shared<Cvar>(L"dx12_shaderMiss3D",   L"dxrMiss.hlsl",                                         CVAR_ARCHIVE);
		shaderHit3D             = std::make_shared<Cvar>(L"dx12_shaderHit3D",    L"dxrHit.hlsl",                                          CVAR_ARCHIVE);
		shaderCommon3D          = std::make_shared<Cvar>(L"dx12_shaderCommon3D", L"dxrCommon.hlsl",                                       CVAR_ARCHIVE);

		zNear2D					= std::make_shared<Cvar>(L"dx12_zNear2D",					0,														CVAR_ARCHIVE);
		zNear3D					= std::make_shared<Cvar>(L"dx12_zNear3D",					4,														CVAR_ARCHIVE);
		zFar2D					= std::make_shared<Cvar>(L"dx12_zFar2D",						99999,													CVAR_ARCHIVE);
		zFar3D					= std::make_shared<Cvar>(L"dx12_zFar3D",						4096,													CVAR_ARCHIVE);

		dxrMaxRecursionDepth    = std::make_shared<Cvar>(L"dx12_dxrMaxRecursionDepth",    2,                                              CVAR_ARCHIVE);

	#if defined(DEBUG) || defined (_DEBUG)
		overlayScale			= std::make_shared<Cvar>(L"dx12_overlayScale",				0.9,													CVAR_ARCHIVE);
	#else
		overlayScale			= std::make_shared<Cvar>(L"dx12_overlayScale",				1.0,													CVAR_ARCHIVE);
	#endif
		overlayBrightness		= std::make_shared<Cvar>(L"dx12_overlayBrightness",			0.1125,													CVAR_ARCHIVE);
		overlayContrast			= std::make_shared<Cvar>(L"dx12_overlayContrast",			1.05,													CVAR_ARCHIVE);

		// Courtesy Kirk Barnes @ http://quake2xp.sourceforge.net/
		xpLitDownloadEnable		= std::make_shared<Cvar>(L"dx12_xpLitDownloadEnable",		false,													CVAR_ARCHIVE);
		xpLitDownloadPath		= std::make_shared<Cvar>(L"dx12_xpLitDownloadPath", L"https://sourceforge.net/p/quake2xp/code/HEAD/tree/",	CVAR_ARCHIVE);
		xpLitPathBaseQ2			= std::make_shared<Cvar>(L"dx12_xpLitPathBaseQ2", L"maps/",												CVAR_ARCHIVE);
		xpLitPathRogue			= std::make_shared<Cvar>(L"dx12_xpLitPathRogue", L"mapsr/",												CVAR_ARCHIVE);
		xpLitPathXatrix			= std::make_shared<Cvar>(L"dx12_xpLitPathXatrix", L"mapsx/",												CVAR_ARCHIVE);

		font					= std::make_shared<Cvar>(L"dx12_font", L"Courier New",											CVAR_ARCHIVE);

		clear					= std::make_shared<Cvar>(L"dx12_clear",						true,													CVAR_ARCHIVE);
		}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::Cvars::Cvar::Cvar(std::wstring name, std::wstring defaultString, unsigned int flags)
{
	LOG_FUNC();

	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		LOG(info) << "<name> " << name << " <string> " << defaultString << " <flags> " << CvarFlagsToString(flags);

		//m_clientMemPtr = std::make_shared<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(defaultString.c_str()), msl::utilities::SafeInt<int>(flags)));
		m_clientMemPtr = ref->client->Cvar_Get(const_cast<char*>(ref->sys->ToString(name).c_str()), const_cast<char*>(ref->sys->ToString(defaultString).c_str()), msl::utilities::SafeInt<int>(flags));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::Cvars::Cvar::Cvar(std::wstring name, float defaultValue, unsigned int flags)
{
	LOG_FUNC();

	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

		//m_clientMemPtr = std::make_shared<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
		m_clientMemPtr = ref->client->Cvar_Get(const_cast<char*>(ref->sys->ToString(name).c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::Cvars::Cvar::Cvar(std::wstring name, double defaultValue, unsigned int flags)
{
	LOG_FUNC();

	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

		//m_clientMemPtr = std::make_shared<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
		m_clientMemPtr = ref->client->Cvar_Get(const_cast<char*>(ref->sys->ToString(name).c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::Cvars::Cvar::Cvar(std::wstring name, int defaultValue, unsigned int flags)
{
	LOG_FUNC();

	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		LOG(info) << "<name> " << name << " <value> " << defaultValue << " <flags> " << CvarFlagsToString(flags);

		//m_clientMemPtr = std::make_shared<cvar_t>(*ref->client->Cvar_Get(const_cast<char*>(name.c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags)));
		m_clientMemPtr = ref->client->Cvar_Get(const_cast<char*>(ref->sys->ToString(name).c_str()), const_cast<char*>(std::to_string(defaultValue).c_str()), msl::utilities::SafeInt<int>(flags));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::Cvars::Cvar::~Cvar()
{
	try
	{
		m_clientMemPtr = nullptr;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

float dx12::Cvars::Cvar::Float()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return  m_clientMemPtr->value;
}

bool dx12::Cvars::Cvar::Bool()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	if (m_clientMemPtr->value > 0.0)
	{
		return true;
	}
	return false;
}

int dx12::Cvars::Cvar::Int()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return msl::utilities::SafeInt<int>(static_cast<int>(round(m_clientMemPtr->value)));
}

unsigned int dx12::Cvars::Cvar::UInt()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return msl::utilities::SafeInt<unsigned int>(static_cast<unsigned int>(round(m_clientMemPtr->value)));
}

double dx12::Cvars::Cvar::Double()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return m_clientMemPtr->value;
}

std::wstring dx12::Cvars::Cvar::String()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return ref->sys->ToWideString(m_clientMemPtr->string);
}

std::wstring dx12::Cvars::Cvar::LatchedString()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return ref->sys->ToWideString(m_clientMemPtr->latched_string);
}

std::wstring dx12::Cvars::Cvar::Name()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return ref->sys->ToWideString(m_clientMemPtr->name);
}

unsigned int dx12::Cvars::Cvar::Flags()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return msl::utilities::SafeInt<unsigned int>(m_clientMemPtr->flags);
}

bool dx12::Cvars::Cvar::Modified()
{
	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		if (m_clientMemPtr->modified)
		{
			return true;
		}
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

inline std::wstring dx12::Cvars::Cvar::CvarFlagsToString(unsigned int flags)
{
	std::wstring flagString = L"";

	try
	{
		if (flags & CVAR_LATCH)
		{
			flagString += L"CVAR_LATCH ";
		}

		if (flags & CVAR_NOSET)
		{
			flagString += L"CVAR_NOSET ";
		}

		if (flags & CVAR_SERVERINFO)
		{
			flagString += L"CVAR_SERVERINFO ";
		}

		if (flags & CVAR_USERINFO)
		{
			flagString += L"CVAR_USERINFO ";
		}

		if (flags & CVAR_ARCHIVE)
		{
			flagString += L"CVAR_ARCHIVE";
		}

		if (flagString == L"")
		{
			flagString = L"__no_flags__";
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return flagString;
}

bool dx12::Cvars::Cvar::InfoValidate(std::wstring value)
{
	LOG_FUNC();

	try
	{
		std::string::size_type found = value.find(L"\\");

		if (found != std::string::npos)
		{
			return false;
		}

		found = value.find(L"\"");
		if (found != std::string::npos)
		{
			return false;
		}

		found = value.find(L";");
		if (found != std::string::npos)
		{
			return false;
		}

		return true;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}


void dx12::Cvars::Cvar::Set(std::wstring value)
{
	LOG_FUNC();

	try
	{
		// Wait for exclusive access
		std::lock_guard<std::mutex> lock(m_ptrAccessMutex);

		// Cvar_Set is costly and poorly implemented when we already have a pointer to the cvar (linear string search)
		// so avoid calling it if at all possible
		if (value.compare(ref->sys->ToWideString(m_clientMemPtr->string)) == 0)
		{
			return;		// not changed
		}

		if (m_clientMemPtr->flags & (CVAR_USERINFO | CVAR_SERVERINFO))
		{
			if (!InfoValidate(value))
			{
				ref->client->Con_Printf(PRINT_ALL, L"Invalid info cvar value\n");
				return;
			}
		}

		if (m_clientMemPtr->flags & CVAR_NOSET)
		{
			ref->client->Con_Printf(PRINT_ALL, ref->sys->ToWideString(m_clientMemPtr->name) + L" is write protected.\n");
			return;
		}

		if (m_clientMemPtr->flags & CVAR_LATCH)
		{
			if (m_clientMemPtr->latched_string != nullptr)
			{
				if (value.compare(ref->sys->ToWideString(m_clientMemPtr->latched_string)) == 0)
				{
					return;
				}
			}
		}

		// Cvar_Set uses some client side functionality for latching, server state, etc that we can't safely replicate here, so we have to use it
		ref->client->Cvar_Set(m_clientMemPtr->name, const_cast<char*>(ref->sys->ToString(value).c_str()));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Cvars::Cvar::Set(float value)
{
	Set(std::to_wstring(value));
}

void dx12::Cvars::Cvar::Set(double value)
{
	Set(std::to_wstring(value));
}

void dx12::Cvars::Cvar::SetModified(bool value)
{
	LOG_FUNC();

	try
	{
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
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Cvars::Cvar::Set(bool value)
{
	Set(std::to_wstring(value));
}

void dx12::Cvars::Cvar::Set(signed int value)
{
	Set(std::to_wstring(value));
}

void dx12::Cvars::Cvar::Set(unsigned int value)
{
	Set(std::to_wstring(value));
}