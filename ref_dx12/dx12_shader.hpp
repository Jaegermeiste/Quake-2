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

#ifndef __DX12_SHADER_HPP__
#define __DX12_SHADER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	typedef std::wstring shaderTarget;

constexpr auto SHADER_TARGET_VERTEX          = L"vs";
constexpr auto SHADER_TARGET_PIXEL           = L"ps";
constexpr auto SHADER_TARGET_COMPUTE         = L"cs";
constexpr auto SHADER_TARGET_DOMAIN          = L"ds";
constexpr auto SHADER_TARGET_GEOMETRY        = L"gs";
constexpr auto SHADER_TARGET_HULL            = L"hs";
constexpr auto SHADER_TARGET_LIBRARY         = L"lib";
constexpr auto SHADER_TARGET_DXR_COMMON      = L"dxr_common_lib";
constexpr auto SHADER_TARGET_DXR_MISS        = L"dxr_miss_lib";
constexpr auto SHADER_TARGET_DXR_HIT         = L"dxr_hit_lib";
constexpr auto SHADER_TARGET_DXR_RAYGEN      = L"dxr_raygen_lib";

constexpr auto SHADER_ENTRY_POINT_VERTEX     = L"VSMain";
constexpr auto SHADER_ENTRY_POINT_PIXEL      = L"PSMain";
constexpr auto SHADER_ENTRY_POINT_COMPUTE    = L"CSMain";
constexpr auto SHADER_ENTRY_POINT_DOMAIN     = L"DSMain";
constexpr auto SHADER_ENTRY_POINT_GEOMETRY   = L"GSMain";
constexpr auto SHADER_ENTRY_POINT_HULL       = L"HSMain";
constexpr auto SHADER_ENTRY_POINT_LIBRARY    = L"LIBMain";
constexpr auto SHADER_ENTRY_POINT_DXR_COMMON = L"Common";
constexpr auto SHADER_ENTRY_POINT_DXR_MISS   = L"Miss";
constexpr auto SHADER_ENTRY_POINT_DXR_HIT    = L"Hit";
constexpr auto SHADER_ENTRY_POINT_DXR_RAYGEN = L"RayGen";

	class Shader 
	{
	private:
		ComPtr<IDxcBlob>			m_dxcBlob = nullptr;
		ComPtr<ID3DBlob>			m_d3dBlob = nullptr;

		ComPtr<IDxcBlob>            DxcCompileShaderFromFile(const std::wstring& filePath, const std::wstring& entryPoint, const std::wstring& target);

		void						OutputCompilerErrorMessages(ComPtr<IDxcResult> result, std::wstring shaderFilename);
		void                        OutputCompilerErrorMessages(ComPtr<ID3DBlob> errorMessage, std::wstring shaderFilename);

	public:
									Shader();

		bool						Compile(std::wstring shaderFilename, shaderTarget target);

		ComPtr<IDxcBlob>			DxcBlob() { return m_dxcBlob; };
		ComPtr<ID3DBlob>			D3DBlob();

		void						Shutdown();
	};
}

#endif // !__DX12_SHADER_HPP__