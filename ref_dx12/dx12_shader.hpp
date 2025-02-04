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

#define SHADER_TARGET_VERTEX	L"vs"
#define SHADER_TARGET_PIXEL		L"ps"
#define SHADER_TARGET_COMPUTE	L"cs"
#define SHADER_TARGET_DOMAIN	L"ds"
#define SHADER_TARGET_GEOMETRY	L"gs"
#define SHADER_TARGET_HULL		L"hs"
#define SHADER_TARGET_LIBRARY	L"lib"

#define SHADER_ENTRY_POINT_VERTEX	L"VSMain"
#define SHADER_ENTRY_POINT_PIXEL	L"PSMain"
#define SHADER_ENTRY_POINT_COMPUTE	L"CSMain"
#define SHADER_ENTRY_POINT_DOMAIN	L"DSMain"
#define SHADER_ENTRY_POINT_GEOMETRY	L"GSMain"
#define SHADER_ENTRY_POINT_HULL		L"HSMain"
#define SHADER_ENTRY_POINT_LIBRARY	L"LIBMain"

	class Shader 
	{
	private:
		ComPtr<ID3DBlob>			m_shaderBlob = nullptr;

		void						OutputShaderErrorMessage(ComPtr<ID3DBlob> errorMessage, std::wstring shaderFilename);

	public:
									Shader();

		bool						Compile(std::wstring shaderFilename, shaderTarget target);

		ComPtr<ID3DBlob>			Blob() { return m_shaderBlob; };

		void						Shutdown();
	};
}

#endif // !__DX12_SHADER_HPP__