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

// Global variable (per MS documentation)
CRITICAL_SECTION CriticalSection;

#if defined(DEBUG) || defined (_DEBUG)
ComPtr<ID3D12Debug>		    d3dDebug		= nullptr;
ComPtr<ID3D12DebugDevice>	d3dDebugDev		= nullptr;
ComPtr<ID3D12InfoQueue>	    d3dInfoQueue	= nullptr;
#endif

/*
===============
dx12::Initialize
===============
*/
bool dx12::Initialize()
{
	LOG_FUNC();

	if (dx12::ref->client != nullptr)
	{
		dx12::ref->client->Con_Printf(PRINT_ALL, L"ref_dx12 version: " REF_VERSION);
	}

	// Initialize the critical section one time only.
	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400))
	{
		return false;
	}

#if defined(DEBUG) || defined (_DEBUG)
	HRESULT hr = E_UNEXPECTED;

	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebug));

	if (FAILED(hr)) {
		LOG(warning) << "Failed to get debug interface: " << GetD3D12ErrorMessage(hr);
	}

	if (d3dDebug != nullptr)
	{
		d3dDebug->EnableDebugLayer();
	}
#endif//DEBUG


	return true;
}

void dx12::DumpD3DDebugMessagesToLog()
{
#if defined(DEBUG) || defined (_DEBUG)
	if (d3dInfoQueue)
	{
		UINT64 numDebugMsgs = d3dInfoQueue->GetNumStoredMessages();

		for (UINT64 i = 0; i < numDebugMsgs; i++)
		{
			// Get the size of the message
			SIZE_T messageLength = 0;
			HRESULT hr = d3dInfoQueue->GetMessage(i, NULL, &messageLength);

			// Allocate space and get the message
			D3D12_MESSAGE * pMessage = (D3D12_MESSAGE*)malloc(messageLength);
			hr = d3dInfoQueue->GetMessage(i, pMessage, &messageLength);

			// Log the message
			LOG(debug) << "Category: " << pMessage->Category << " Severity: " << pMessage->Severity << " Description: " << pMessage->pDescription;
		}

		d3dInfoQueue->ClearStoredMessages();
	}
#endif
}

/*
===============
dx12::Shutdown
===============
*/
void dx12::Shutdown()
{
	LOG_FUNC();

	// Release resources used by the critical section object.
	DeleteCriticalSection(&CriticalSection);

#if defined(DEBUG) || defined (_DEBUG)
	if (d3dDebugDev != nullptr)
	{
		d3dDebugDev->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL);
	}

	SAFE_RELEASE(d3dInfoQueue);

	SAFE_RELEASE(d3dDebugDev);

	SAFE_RELEASE(d3dDebug);
#endif

}