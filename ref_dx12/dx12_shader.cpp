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
2025 Bleeding Eye Studios
*/

#include "dx12_local.hpp"


dx12::Shader::Shader()
{
	LOG_FUNC();

	m_dxcBlob = nullptr;
}

ComPtr<IDxcBlob> dx12::Shader::DxcCompileShaderFromFile(const std::wstring& filePath, const std::wstring& entryPoint, const std::wstring& target)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	ComPtr<IDxcBlob> shaderBlob = nullptr;

	try {
		// Initialize DXC components
		ComPtr<IDxcCompiler3> compiler;
		ComPtr<IDxcUtils> utils;
		ComPtr<IDxcIncludeHandler> includeHandler;

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create DXC compiler: " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create DXC Utils: " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		hr = utils->CreateDefaultIncludeHandler(&includeHandler);

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create DXC Include Handler: " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		// Load the shader file
		ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
		hr = utils->LoadFile(filePath.c_str(), nullptr, &sourceBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Unable to load shader file '" << filePath << "': " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		if (!sourceBlob)
		{
			LOG(error) << L"Failed to load shader file '" << filePath << "'.";
			return nullptr;
		}

#if defined(DEBUG) || defined (_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		// Setup compile arguments
		std::vector<LPCWSTR> arguments = {
			filePath.c_str(),    // Shader file path
			L"-E", entryPoint.c_str(), // Entry point
			L"-T", target.c_str(), // Target profile (lib_6_3 for DXR)
			L"-Zi",  // Debug info
			L"-Qstrip_debug", // Strip debug info from final binary
			L"-Qstrip_reflect",         // Strip reflection data
			L"-Od", // Disable optimizations for easier debugging
			L"-Zpr" // Row-major matrix layout
		};
#else
		// Setup compile arguments
		std::vector<LPCWSTR> arguments = {
			filePath.c_str(),    // Shader file path
			L"-E", entryPoint.c_str(), // Entry point
			L"-T", target.c_str(), // Target profile (lib_6_3 for DXR)
			L"-Qstrip_debug", // Strip debug info from final binary
			L"-Qstrip_reflect",         // Strip reflection data
			L"-O3", // Max optimization level
			L"-Zpr" // Row-major matrix layout
		};
#endif

		// Compile shader
		DxcBuffer sourceBuffer = {};
		sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
		sourceBuffer.Size = sourceBlob->GetBufferSize();
		sourceBuffer.Encoding = DXC_CP_UTF8;

		ComPtr<IDxcResult> result = nullptr;
		hr = compiler->Compile(&sourceBuffer, arguments.data(), static_cast<UINT32>(arguments.size()), includeHandler.Get(), IID_PPV_ARGS(&result));

		// Check for compilation errors
		OutputCompilerErrorMessages(result, filePath);

		if (FAILED(hr))
		{
			LOG(error) << "Unable to compile shader: " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		// Retrieve compiled shader
		hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
		if (FAILED(hr))
		{
			LOG(error) << "Unable to obtain shader output: " << GetD3D12ErrorMessage(hr);
		}
		if (!shaderBlob)
		{
			LOG(error) << L"Unable to obtain shader output: " << filePath;
			return nullptr;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return shaderBlob;
}

bool dx12::Shader::Compile(std::wstring fileName, shaderTarget target)
{
	LOG_FUNC();

	std::wstring entryPoint = L"";

	HRESULT		hr = E_UNEXPECTED;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT shaderCompileFlags = 0;
#endif
	
	try
	{
		const std::wstring gameDir = ref->client->FS_Gamedir();
		std::wstring absoluteGameDir = ref->client->FS_GamedirAbsolute();
		std::wstring currentWorkingDirectory = ref->sys->GetCurrentWorkingDirectory();

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
		else if (target == SHADER_TARGET_DXR_COMMON)
		{
			entryPoint = SHADER_ENTRY_POINT_DXR_COMMON;
		}
		else if (target == SHADER_TARGET_DXR_MISS)
		{
			entryPoint = SHADER_ENTRY_POINT_DXR_MISS;
		}
		else if (target == SHADER_TARGET_DXR_HIT)
		{
			entryPoint = SHADER_ENTRY_POINT_DXR_HIT;
		}
		else if (target == SHADER_TARGET_DXR_RAYGEN)
		{
			entryPoint = SHADER_ENTRY_POINT_DXR_RAYGEN;
		}
		else
		{
			LOG(error) << "Invalid shader target " << target << " provided.";
			return false;
		}

		if ((target == SHADER_TARGET_DXR_COMMON) ||
			(target == SHADER_TARGET_DXR_MISS) ||
			(target == SHADER_TARGET_DXR_HIT) ||
			(target == SHADER_TARGET_DXR_RAYGEN))
		{
			target = L"lib_6_6";
		}
		else
		{
			// Build appropriate targets for feature level
			switch (ref->sys->dx->FeatureLevel())
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
				shaderCompileFlags |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
			}
		}

		if (absoluteGameDir != currentWorkingDirectory)
		{
			LOG(info) << "Current working directory " << currentWorkingDirectory << " is not the gamedir " << absoluteGameDir << ", prepending relative directory to fileName " << fileName;
			fileName = gameDir + L"/" + fileName;
		}

		// Compile the shader code.
		LOG(info) << "Compiling Shader File " << fileName << " with target " << target << " and entry point " << entryPoint;

		if (target.find(L"_6_") != std::string::npos)
		{
			m_dxcBlob = DxcCompileShaderFromFile(fileName, entryPoint, target);

			if (!m_dxcBlob)
			{
				LOG(error) << "Failed to obtain shader buffer";
				return false;
			}
		}
		else
		{
			ComPtr<ID3DBlob>	errorMessage = nullptr;

			hr = D3DCompileFromFile(fileName.c_str(), NULL, NULL, ToString(entryPoint).c_str(), ToString(target).c_str(), shaderCompileFlags, 0, &m_d3dBlob, &errorMessage);

			if (FAILED(hr))
			{
				// If the shader failed to compile it should have writen something to the error message.
				if (errorMessage)
				{
					OutputCompilerErrorMessages(errorMessage, fileName);
				}
				else
				{
					LOG(error) << "Windows Error: " << GetD3D12ErrorMessage(hr);
				}

				if (m_d3dBlob)
				{
					SAFE_RELEASE(m_d3dBlob);
				}

				return false;
			}
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


ComPtr<ID3DBlob> dx12::Shader::D3DBlob()
{
	LOG_FUNC();

	try
	{
		if (m_d3dBlob)
		{
			return m_d3dBlob;
		}
		else if (m_dxcBlob)
		{
			if (FAILED(D3DCreateBlob(m_dxcBlob->GetBufferSize(), &m_d3dBlob)))
			{
				LOG(error) << "Failed to create ID3DBlob";
				return nullptr;
			}

			memcpy(m_d3dBlob->GetBufferPointer(), m_dxcBlob->GetBufferPointer(), m_dxcBlob->GetBufferSize());
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return m_d3dBlob;
}

void dx12::Shader::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	try
	{
		SAFE_RELEASE(m_dxcBlob);

		SAFE_RELEASE(m_d3dBlob);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	LOG(info) << "Shutdown complete.";
}


void dx12::Shader::OutputCompilerErrorMessages(ComPtr<IDxcResult> result = nullptr, std::wstring shaderFilename = L"")
{
	LOG_FUNC();

	try {
		if (result)
		{
			ComPtr<IDxcBlobUtf8> errors = nullptr;
			HRESULT hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

			if (FAILED(hr))
			{
				LOG(error) << "Unable to obtain shader '" << shaderFilename << "' compilation errors: " << GetD3D12ErrorMessage(hr);
			}
			else if (errors && errors->GetStringLength() > 0)
			{
				LOG(error) << L"Shader '" << shaderFilename << "' Compilation Errors: " << errors->GetStringPointer();
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

void dx12::Shader::OutputCompilerErrorMessages(ComPtr<ID3DBlob> errorMessage = nullptr, std::wstring shaderFilename = L"")
{
	LOG_FUNC();
	
	try {
		if (errorMessage)
		{
			char* compileErrors = nullptr;
			unsigned long bufferSize = 0;
			std::wstring errMsg = L"";

			// Get a pointer to the error message text buffer.
			compileErrors = (char*)(errorMessage->GetBufferPointer());

			// Get the length of the message.
			bufferSize = errorMessage->GetBufferSize();

			if (bufferSize > 0)
			{
				LOG(error) << L"Shader '" << shaderFilename << "' Compilation Errors: " << compileErrors;
			}

			// Release the error message.
			SAFE_RELEASE(errorMessage);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}