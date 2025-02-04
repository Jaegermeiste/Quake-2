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

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Contents courtesy http://rastertek.com/dx12s2tut05.html !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include "dx12_local.hpp"


dx12::Shader::Shader()
{
	LOG_FUNC();
}

bool dx12::Shader::Compile(std::wstring fileName, shaderTarget target)
{
	LOG_FUNC();

	ComPtr<ID3DBlob>	errorMessage = nullptr;
	std::wstring entryPoint = L"";
	HRESULT		hr = E_UNEXPECTED;
	const std::wstring gameDir = ref->client->FS_Gamedir();
	std::wstring absoluteGameDir = ref->client->FS_GamedirAbsolute();
	std::wstring currentWorkingDirectory = ref->sys->GetCurrentWorkingDirectory();

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT shaderCompileFlags = 0;
#endif

	// Figure out the entry point
	if (target == SHADER_TARGET_VERTEX)
	{
		entryPoint = SHADER_ENTRY_POINT_VERTEX;
	}
	else if (target == SHADER_TARGET_PIXEL)
	{
		entryPoint = SHADER_ENTRY_POINT_PIXEL;
	}
	else if (target == SHADER_TARGET_COMPUTE)
	{
		entryPoint = SHADER_ENTRY_POINT_COMPUTE;
	}
	else if (target == SHADER_TARGET_DOMAIN)
	{
		entryPoint = SHADER_ENTRY_POINT_DOMAIN;
	}
	else if (target == SHADER_TARGET_GEOMETRY)
	{
		entryPoint = SHADER_ENTRY_POINT_GEOMETRY;
	}
	else if (target == SHADER_TARGET_HULL)
	{
		entryPoint = SHADER_ENTRY_POINT_HULL;
	}
	else if (target == SHADER_TARGET_LIBRARY)
	{
		entryPoint = SHADER_ENTRY_POINT_LIBRARY;
	}
	else
	{
		LOG(error) << "Invalid shader target " << target << " provided.";
		return false;
	}

	// Build appropriate targets for feature level
	switch (ref->sys->dx->FeatureLevel())		// FIXME
	{
	case D3D_FEATURE_LEVEL_12_2:
	case D3D_FEATURE_LEVEL_12_1:
	case D3D_FEATURE_LEVEL_12_0:
		target += L"_5_1";
		break;
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
		target += L"_5_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_1 is not supported in D3D_FEATURE_LEVEL_10_1";
			return false;
		}
		target += L"_4_1";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0 is not supported in D3D_FEATURE_LEVEL_10_0";
			return false;
		}
		target += L"_4_0";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0_level_9_3 is not supported in D3D_FEATURE_LEVEL_9_3";
			return false;
		}
		target += L"_4_0_level_9_3";
		break;
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
	default:
		if ((target != SHADER_TARGET_VERTEX) && (target != SHADER_TARGET_PIXEL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0_level_9_1 is not supported in D3D_FEATURE_LEVEL_9_1 or _9_2";
			return false;
		}
		target += L"_4_0_level_9_1";
		shaderCompileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
	}

	if (absoluteGameDir != currentWorkingDirectory)
	{
		LOG(info) << "Current working directory " << currentWorkingDirectory << " is not the gamedir " << absoluteGameDir << ", prepending relative directory to fileName " << fileName;
		fileName = gameDir + L"/" + fileName;
	}
	
	// Compile the shader code.
	LOG(info) << "Compiling Shader File " << fileName << " with target " << target << " and entry point " << entryPoint;
	
	hr = D3DCompileFromFile(fileName.c_str(), NULL, NULL, ref->sys->ToString(entryPoint).c_str(), ref->sys->ToString(target).c_str(), shaderCompileFlags, 0, &m_shaderBlob, &errorMessage);

	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, fileName);
		}
		else
		{
			LOG(error) << "Windows Error: " << GetD3D12ErrorMessage(hr);
		}

		if (m_shaderBlob)
		{
			SAFE_RELEASE(m_shaderBlob);
		}

		return false;
	}

	if (!m_shaderBlob)
	{
		LOG(error) << "Failed to obtain shader buffer";
		return false;
	}

	return true;
}


void dx12::Shader::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	SAFE_RELEASE(m_shaderBlob);
	
	LOG(info) << "Shutdown complete.";
}


void dx12::Shader::OutputShaderErrorMessage(ComPtr<ID3DBlob> errorMessage, std::wstring shaderFilename)
{
	if (errorMessage)
	{
		char* compileErrors = nullptr;
		unsigned long bufferSize = 0;
		std::wstring errMsg = L"";

		// Get a pointer to the error message text buffer.
		compileErrors = (char*)(errorMessage->GetBufferPointer());

		// Get the length of the message.
		bufferSize = errorMessage->GetBufferSize();

		LOG(error) << compileErrors;

		// Release the error message.
		SAFE_RELEASE(errorMessage);
	}
}