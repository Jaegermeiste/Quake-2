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

#ifndef __STDAFX_H__
#define __STDAFX_H__
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// STL Includes
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <exception>

// Boost Includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/keyword.hpp>

// Windows Includes
#include <windows.h>
#include <mmsystem.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <shellapi.h>
#include <SafeInt.h>
#include <ppl.h>

/*
#ifdef _DEBUG
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn904627
#include <TraceLoggingProvider.h>  // The native TraceLogging API
#include <winmeta.h> // for WINEVENT_LEVEL_* definitions

// Forward-declare the loggingProvider variable that you will use for tracing in this component
TRACELOGGING_DECLARE_PROVIDER(loggingProvider);

// {53430B16-C35C-428D-834F-B1C86D38E499}
#define TRACELOGGING_GUID_RAW		0x53430b16, 0xc35c, 0x428d, 0x83, 0x4f, 0xb1, 0xc8, 0x6d, 0x38, 0xe4, 0x99

#ifdef DEFINE_GUID

DEFINE_GUID(TRACELOGGING_DEBUG_QUAKE2_DX11 , 0x53430b16, 0xc35c, 0x428d, 0x83, 0x4f, 0xb1, 0xc8, 0x6d, 0x38, 0xe4, 0x99);
#endif

#define TRACELOGGING_GUID			(TRACELOGGING_GUID_RAW)

#define TRACELOGGING_PROVIDER_NAME	"Quake2DX11LoggingProvider"
#endif
*/

// DirectXTK11
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
//#include "ResourceUploadBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"

//#include "d3dx11.h"

// DirectXTex
#include "DirectXTex.h"
//#include "DirectXTexEXR.h"

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}
}

#endif//__STDAFX_H__