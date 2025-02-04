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

#ifndef __DX12_LOCAL_HPP__
#define __DX12_LOCAL_HPP__
#pragma once

#include "stdafx.h"

#include "../client/ref.h"
#include "../win32/winquake.h"

#pragma warning(default:4100)	// Unreferenced formal parameter
#pragma warning(default:4242)	// Possible loss of data
#pragma warning(default:4365)	// Signed/unsigned mismatch
#pragma warning(default:4820)	// Padding

extern	unsigned short	BigUShort(unsigned short l);
extern	unsigned short	LittleUShort(unsigned short l);
extern	unsigned int	BigULong(unsigned int l);
extern	unsigned int	LittleULong(unsigned int l);

#define	REF_VERSION	"DX12 " __DATE__ " " __TIME__

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

// Courtesy https://stackoverflow.com/questions/195975/how-to-make-a-char-string-from-a-c-macros-value
#define QUOTE(name) L#name
#define STR(macro) QUOTE(macro)

// Handles
typedef int			qhandle_t;
typedef std::size_t dxhandle_t;

typedef enum rserr_e
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

typedef struct viddef_s
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

typedef enum imagetype_e
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

/*typedef struct image_s
{
	char				name[MAX_QPATH];				// game path, including extension
	imagetype_t			type;
	int					width, height;					// source image
	int					upload_width, upload_height;	// after power of two and picmip
	int					registration_sequence;			// 0 = free
	struct msurface_s	*texturechain;					// for sort-by-texture world drawing
	int					texnum;							// gl texture binding
	float				sl, tl, sh, th;					// 0,0 - 1,1 unless part of the scrap
	qboolean			scrap;
	qboolean			has_alpha;

	qboolean			paletted;

	byte				padding[1];
} image_t;*/

constexpr auto EMA_ALPHA = 0.9;

#include "dx12_log.hpp"


// Detect if a type is a COM interface (Direct or Indirect Inheritance from IUnknown)
template <typename T, typename = void>
struct is_com_interface : std::false_type {};

template <typename T>
struct is_com_interface<T, std::enable_if_t<std::is_base_of_v<IUnknown, std::remove_pointer_t<T>>>> : std::true_type {};

// Detect if a type is a ComPtr holding an IUnknown-derived object
template <typename T>
struct is_com_ptr : std::false_type {};

template <typename T>
struct is_com_ptr<Microsoft::WRL::ComPtr<T>> : is_com_interface<T> {};

// Overload 1: **Raw Pointers**
template <typename T>
std::enable_if_t<!is_com_interface<T>::value&& std::is_pointer_v<T>>
SAFE_RELEASE(T& ptr) {
    if (ptr) {
        try {
            delete ptr;
            ptr = nullptr;
            LOG(debug) << "[SAFE_RELEASE] Raw Pointer delete successful.";
        }
        catch (const std::exception& e) {
            LOG(error) << "[SAFE_RELEASE] Exception while deleting raw pointer: " << e.what();
        }
        catch (...) {
            LOG(error) << "[SAFE_RELEASE] Unknown exception occurred while deleting raw pointer!";
        }
    }
}

// Overload 2: **Raw COM Pointers**
template <typename T>
std::enable_if_t<is_com_interface<T>::value&& std::is_pointer_v<T>>
SAFE_RELEASE(T& ptr) {
    if (ptr) {
        try {
            ULONG refCount = ptr->Release();
            ptr = nullptr;
            LOG(debug) << "[SAFE_RELEASE] COM object released, remaining ref count: " << refCount;
        }
        catch (const std::exception& e) {
            LOG(error) << "[SAFE_RELEASE] Exception while releasing COM object: " << e.what();
        }
        catch (...) {
            LOG(error) << "[SAFE_RELEASE] Unknown exception occurred while releasing COM object!";
        }
    }
}

// Overload 3: **ComPtr<T>**
template <typename T>
std::enable_if_t<is_com_ptr<T>::value>
SAFE_RELEASE(T& ptr) {
    try {
        ptr.Reset();
        LOG(debug) << "[SAFE_RELEASE] ComPtr Reset successful.";
    }
    catch (const std::exception& e) {
        LOG(error) << "[SAFE_RELEASE] Exception while resetting ComPtr: " << e.what();
    }
    catch (...) {
        LOG(error) << "[SAFE_RELEASE] Unknown exception occurred while resetting ComPtr!";
    }
}

// Overload 4: **unique_ptr<T> and shared_ptr<T>**
template <typename T>
void SAFE_RELEASE(std::unique_ptr<T>& ptr) {
    try {
        ptr.reset();
        LOG(debug) << "[SAFE_RELEASE] unique_ptr reset successful.";
    }
    catch (const std::exception& e) {
        LOG(error) << "[SAFE_RELEASE] Exception while resetting unique_ptr: " << e.what();
    }
    catch (...) {
        LOG(error) << "[SAFE_RELEASE] Unknown exception occurred while resetting unique_ptr!";
    }
}

template <typename T>
void SAFE_RELEASE(std::shared_ptr<T>& ptr) {
    try {
        ptr.reset();
        LOG(debug) << "[SAFE_RELEASE] shared_ptr reset successful.";
    }
    catch (const std::exception& e) {
        LOG(error) << "[SAFE_RELEASE] Exception while resetting shared_ptr: " << e.what();
    }
    catch (...) {
        LOG(error) << "[SAFE_RELEASE] Unknown exception occurred while resetting shared_ptr!";
    }
}


static std::wstring FormatHResultMessage(HRESULT hr) {
    try {
        wchar_t* msgBuffer = nullptr;
        DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&msgBuffer, 0, nullptr);

        if (size == 0 || msgBuffer == nullptr) {
            return L"Unknown error (FormatMessageW failed)";
        }

        std::wstring message(msgBuffer);
        LocalFree(msgBuffer);
        return message;

    }
    catch (const std::exception& e) {
        LOG(error) << "Exception in FormatHResultMessage: " << e.what();
        return L"Unknown error (Exception occurred)";
    }
}

static std::wstring GetD3D12ErrorMessage(HRESULT hr) {
    static const std::unordered_map<HRESULT, std::wstring> errorMap = {
        {DXGI_ERROR_DEVICE_REMOVED,  L"DXGI_ERROR_DEVICE_REMOVED: The GPU was removed or reset (Check GPU overheating, driver crashes)."},
        {DXGI_ERROR_DEVICE_HUNG,     L"DXGI_ERROR_DEVICE_HUNG: The GPU driver crashed (Often caused by invalid GPU commands)."},
        {DXGI_ERROR_DEVICE_RESET,    L"DXGI_ERROR_DEVICE_RESET: The GPU was reset due to a timeout (TDR issue, consider debugging)."},
        {DXGI_ERROR_DRIVER_INTERNAL_ERROR, L"DXGI_ERROR_DRIVER_INTERNAL_ERROR: A serious driver issue occurred (Try updating GPU drivers)."},
        {DXGI_ERROR_FRAME_STATISTICS_DISJOINT, L"DXGI_ERROR_FRAME_STATISTICS_DISJOINT: Frame stats are invalid, likely due to vsync state changes."},
        {DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, L"DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: The output display is already in use."},
        {DXGI_ERROR_INVALID_CALL,    L"DXGI_ERROR_INVALID_CALL: The function was called incorrectly (Check parameters)."},
        {DXGI_ERROR_MORE_DATA,       L"DXGI_ERROR_MORE_DATA: The buffer is too small, request a larger one."},
        {DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: The resource is unavailable (GPU too busy, or another fullscreen app is active)."},
        {DXGI_ERROR_NOT_FOUND,       L"DXGI_ERROR_NOT_FOUND: The requested item was not found (Check resource names, adapters, or outputs)."},
        {DXGI_ERROR_UNSUPPORTED,     L"DXGI_ERROR_UNSUPPORTED: The requested operation is not supported by the GPU/driver."},
        {DXGI_ERROR_ACCESS_DENIED,   L"DXGI_ERROR_ACCESS_DENIED: The operation was denied (Check permissions, security policies)."},
        {DXGI_ERROR_ACCESS_LOST,     L"DXGI_ERROR_ACCESS_LOST: The device lost access to the GPU (TDR issue, GPU removed)."},
        {DXGI_ERROR_NONEXCLUSIVE,     L"DXGI_ERROR_NONEXCLUSIVE: A global counter resource was in use, and the specified counter cannot be used by this Direct3D device at this time."},
        {DXGI_ERROR_WAIT_TIMEOUT,    L"DXGI_ERROR_WAIT_TIMEOUT: A timeout occurred while waiting for GPU execution."},
        {DXGI_ERROR_SESSION_DISCONNECTED, L"DXGI_ERROR_SESSION_DISCONNECTED: The remote desktop session was disconnected."},
        {DXGI_ERROR_WAS_STILL_DRAWING, L"DXGI_ERROR_WAS_STILL_DRAWING: The GPU was busy at the moment when the call was made, and the call was neither executed nor scheduled."},
        {DXGI_ERROR_ALREADY_EXISTS, L"DXGI_ERROR_ALREADY_EXISTS: The desired element already exists."},
        {DXGI_ERROR_NAME_ALREADY_EXISTS, L"DXGI_ERROR_NAME_ALREADY_EXISTS: The application is trying to create a shared handle using a name that is already associated with some other resource."},
        {DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, L"DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: The application's remote device has been removed due to session disconnect or network disconnect."},
        {DXGI_ERROR_REMOTE_OUTOFMEMORY, L"DXGI_ERROR_REMOTE_OUTOFMEMORY: The device has been removed during a remote session because the remote computer ran out of memory."},
        {DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, L"DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: DXGI is unable to provide content protection on the swapchain."},
        {DXGI_ERROR_REMOTE_OUTOFMEMORY, L"DXGI_ERROR_REMOTE_OUTOFMEMORY: The device has been removed during a remote session because the remote computer ran out of memory."},
        {DXGI_ERROR_MPO_UNPINNED, L"DXGI_ERROR_MPO_UNPINNED: The allocation of the MPO plane has been unpinned."},
        {DXGI_DDI_ERR_WASSTILLDRAWING, L"DXGI_DDI_ERR_WASSTILLDRAWING: The GPU was busy when the operation was requested."},
        {DXGI_DDI_ERR_UNSUPPORTED, L"DXGI_DDI_ERR_UNSUPPORTED: The driver has rejected the creation of this resource."},
        {DXGI_DDI_ERR_NONEXCLUSIVE, L"DXGI_DDI_ERR_NONEXCLUSIVE: The GPU counter was in use by another process or d3d device when application requested access to it."},
        {D3D12_ERROR_ADAPTER_NOT_FOUND, L"D3D12_ERROR_ADAPTER_NOT_FOUND: No compatible GPU adapter was found."},
        {D3D12_ERROR_DRIVER_VERSION_MISMATCH, L"D3D12_ERROR_DRIVER_VERSION_MISMATCH: The GPU driver version does not match Direct3D 12 requirements."},
        {D3D12_ERROR_INVALID_REDIST, L"D3D12_ERROR_INVALID_REDIST: The D3D12 SDK version configuration of the host exe is invalid."},
        {DXGI_ERROR_SDK_COMPONENT_MISSING, L"DXGI_ERROR_SDK_COMPONENT_MISSING: A necessary DXGI SDK component is missing."},
        {DXGI_ERROR_NOT_CURRENT, L"DXGI_ERROR_NOT_CURRENT: The DXGI objects that the application has created are no longer current & need to be recreated for this operation to be performed."},
        {DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY, L"DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY: Insufficient HW protected memory exits for proper function."},
        {DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION, L"DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION: Creating this device would violate the process's dynamic code policy."},
        {DXGI_ERROR_NON_COMPOSITED_UI, L"DXGI_ERROR_NON_COMPOSITED_UI: The operation failed because the compositor is not in control of the output."},
        {DXGI_STATUS_UNOCCLUDED, L"DXGI_STATUS_UNOCCLUDED: The swapchain has become unoccluded."},
        {DXGI_STATUS_DDA_WAS_STILL_DRAWING, L"DXGI_STATUS_DDA_WAS_STILL_DRAWING:The adapter did not have access to the required resources to complete the Desktop Duplication Present() call, the Present() call needs to be made again."},
        {DXGI_ERROR_MODE_CHANGE_IN_PROGRESS, L"DXGI_ERROR_MODE_CHANGE_IN_PROGRESS: An on-going mode change prevented completion of the call."},
        {DXGI_STATUS_PRESENT_REQUIRED, L"DXGI_STATUS_PRESENT_REQUIRED: The present succeeded but the caller should present again on the next V-sync, even if there are no changes to the content."},
        {DXGI_ERROR_CACHE_CORRUPT, L"DXGI_ERROR_CACHE_CORRUPT: The shader cache is corrupt and either could not be opened or could not be reset."},
        {DXGI_ERROR_CACHE_FULL, L"DXGI_ERROR_CACHE_FULL: This entry would cause the cache to exceed its quota."},
        {DXGI_ERROR_CACHE_HASH_COLLISION, L"DXGI_ERROR_CACHE_HASH_COLLISION: A cache entry was found, but the key provided does not match the key stored in the entry."},
        {PRESENTATION_ERROR_LOST, L"PRESENTATION_ERROR_LOST: The presentation manager has been lost and can no longer be used."},
        {DXCORE_ERROR_EVENT_NOT_UNREGISTERED, L"DXCORE_ERROR_EVENT_NOT_UNREGISTERED: The application failed to unregister from an event it registered for."},

        // Common System Errors
        {HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), L"ERROR_FILE_NOT_FOUND: The system cannot find the specified file."},
        {HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND), L"ERROR_PATH_NOT_FOUND: The system cannot find the specified path."},
        {HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"ERROR_ACCESS_DENIED: Access was denied. (Check file permissions)"},
        {HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE), L"ERROR_INVALID_HANDLE: The specified handle is invalid."},
        {HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY), L"ERROR_NOT_ENOUGH_MEMORY: Not enough memory available to process this request."},
        {HRESULT_FROM_WIN32(ERROR_DISK_FULL), L"ERROR_DISK_FULL: The disk is full."},
        {HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION), L"ERROR_SHARING_VIOLATION: The process cannot access the file because another process is using it."},
        {HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER), L"ERROR_INVALID_PARAMETER: A parameter is incorrect."},
        {HRESULT_FROM_WIN32(ERROR_BAD_LENGTH), L"ERROR_BAD_LENGTH: Specified length is invalid."},

        // Common D3D12 Errors
        {E_FAIL,                     L"E_FAIL: Unspecified failure, check debug logs."},
        {E_INVALIDARG,               L"E_INVALIDARG: Invalid argument passed to function."},
        {E_OUTOFMEMORY,              L"E_OUTOFMEMORY: Not enough memory available for the operation."},
        {E_ACCESSDENIED,             L"E_ACCESSDENIED: Access was denied (Check permissions)."},
        {E_NOTIMPL,                  L"E_NOTIMPL: The requested functionality is not implemented."},
        {E_NOINTERFACE,              L"E_NOINTERFACE: The requested COM interface is not supported."},
        {E_POINTER,                  L"E_POINTER: Null pointer exception, ensure pointers are initialized."},
        {E_UNEXPECTED,               L"E_UNEXPECTED: Unexpected error occurred (Check logs and debug output)."},
        {S_FALSE,                    L"S_FALSE: The function succeeded but returned a non-critical warning."},
        {S_OK,                       L"S_OK: The operation succeeded without errors."}
    };

    try {        
        // Return mapped error message if found
        auto it = errorMap.find(hr);
        if (it != errorMap.end()) {
            return it->second;
        }

        // Fallback to system message if error code is unknown
        return FormatHResultMessage(hr);

    }
    catch (const std::exception& e) {
        LOG(error) << "Exception in GetD3D12ErrorMessage: " << e.what();
        return L"Unknown error (Exception occurred)";
    }
}

extern CRITICAL_SECTION CriticalSection;
#ifdef _DEBUG
extern ComPtr<ID3D12Debug> d3dDebug;
extern ComPtr<ID3D12DebugDevice> d3dDebugDev;
extern ComPtr<ID3D12InfoQueue> d3dInfoQueue;
#endif

namespace dx12 {
	typedef __declspec(align(16)) struct Vertex2D_s
	{
		DirectX::XMFLOAT4A		position;
		DirectX::XMVECTORF32	color;
		DirectX::XMFLOAT2A		texCoord;
	} Vertex2D;
}


#include "dx12_cvar.hpp"
#include "dx12_descriptorHeap.hpp"
#include "dx12_commandList.hpp"
#include "dx12_resource.hpp"
#include "dx12_shader.hpp"
#include "dx12_web.hpp"
#include "dx12_resourceBuffer.hpp"
#include "dx12_constantBuffer2D.hpp"
#include "dx12_vertexBuffer.hpp"
#include "dx12_indexBuffer.hpp"
#include "dx12_indexedGeometry.hpp"
#include "dx12_testTriangle.hpp"
#include "dx12_quad2D.hpp"
#include "dx12_draw.hpp"
#include "dx12_subsystem2D.hpp"
#include "dx12_subsystem3D.hpp"
#include "dx12_subsystemText.hpp"
#include "dx12_texture2D.hpp"
#include "dx12_image.hpp"
#include "dx12_dx.hpp"
#include "dx12_system.hpp"
#include "dx12_client.hpp"
#include "dx12_model.hpp"
#include "dx12_light.hpp"
#include "dx12_xplit.hpp"
#include "dx12_bsp.hpp"
#include "dx12_map.hpp"
#include "dx12_media.hpp"
#include "dx12_ref.hpp"

namespace dx12
{
	// Functions
	bool			Initialize();
	void			Shutdown();

	void			DumpD3DDebugMessagesToLog();
}

#endif // !__DX12_LOCAL_HPP__
