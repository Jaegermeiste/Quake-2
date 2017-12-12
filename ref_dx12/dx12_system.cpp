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

#include "dx12_local.hpp"

dx12::System* sys = nullptr;

D3D_FEATURE_LEVEL FeatureLevelForString(std::string featureLevelString)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;

	std::map<std::string, D3D_FEATURE_LEVEL> featureLevelMap;
	featureLevelMap["D3D_FEATURE_LEVEL_12_1"] = D3D_FEATURE_LEVEL_12_1;
	featureLevelMap["D3D_FEATURE_LEVEL_12_0"] = D3D_FEATURE_LEVEL_12_0;
	featureLevelMap["D3D_FEATURE_LEVEL_11_1"] = D3D_FEATURE_LEVEL_11_1;
	featureLevelMap["D3D_FEATURE_LEVEL_11_0"] = D3D_FEATURE_LEVEL_11_0;
	featureLevelMap["D3D_FEATURE_LEVEL_10_1"] = D3D_FEATURE_LEVEL_10_1;
	featureLevelMap["D3D_FEATURE_LEVEL_10_0"] = D3D_FEATURE_LEVEL_10_0;
	featureLevelMap["D3D_FEATURE_LEVEL_9_3"] = D3D_FEATURE_LEVEL_9_3;
	featureLevelMap["D3D_FEATURE_LEVEL_9_2"] = D3D_FEATURE_LEVEL_9_2;
	featureLevelMap["D3D_FEATURE_LEVEL_9_1"] = D3D_FEATURE_LEVEL_9_1;

	auto search = featureLevelMap.find(featureLevelString);
	if (search != featureLevelMap.end()) 
	{
		featureLevel = search->second;
	}

	return featureLevel;
}

void dx12::System::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), FeatureLevelForString(ref->cvars->featureLevel->String()), _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
