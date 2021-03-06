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

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Contents courtesy http://rastertek.com/dx11s2tut05.html !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include "dx11_local.hpp"


dx11::Shader::Shader()
{
	LOG_FUNC();

	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_sampleState = nullptr;
}

bool dx11::Shader::CompileShader(ID3D11Device* device, std::string fileName, shaderTarget target, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements)
{
	LOG_FUNC();

	ID3DBlob*	shaderBuffer = nullptr;
	ID3DBlob*	errorMessage = nullptr;
	std::string entryPoint = "";
	HRESULT		hr = E_UNEXPECTED;
	const std::string gameDir = ref->client->FS_Gamedir();
	std::string absoluteGameDir = ref->client->FS_GamedirAbsolute();
	std::string currentWorkingDirectory = ref->sys->GetCurrentWorkingDirectory();
	UINT		shaderFlags = 0;

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
	switch (device->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_12_1:
	case D3D_FEATURE_LEVEL_12_0:
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
		target += "_5_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_1 is not supported in D3D_FEATURE_LEVEL_10_1";
			return false;
		}
		target += "_4_1";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0 is not supported in D3D_FEATURE_LEVEL_10_0";
			return false;
		}
		target += "_4_0";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		if ((target == SHADER_TARGET_DOMAIN) || (target == SHADER_TARGET_HULL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0_level_9_3 is not supported in D3D_FEATURE_LEVEL_9_3";
			return false;
		}
		target += "_4_0_level_9_3";
		break;
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
	default:
		if ((target != SHADER_TARGET_VERTEX) && (target != SHADER_TARGET_PIXEL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_4_0_level_9_1 is not supported in D3D_FEATURE_LEVEL_9_1 or _9_2";
			return false;
		}
		target += "_4_0_level_9_1";
		shaderFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
	}

	if (absoluteGameDir != currentWorkingDirectory)
	{
		LOG(info) << "Current working directory " << currentWorkingDirectory << " is not the gamedir " << absoluteGameDir << ", prepending relative directory to fileName " << fileName;
		fileName = gameDir + "/" + fileName;
	}
	
	// Compile the shader code.
	LOG(info) << "Compiling Shader File " << fileName << " with target " << target << " and entry point " << entryPoint;
	
	hr = D3DCompileFromFile(ref->sys->ToWideString(fileName).c_str(), NULL, NULL, entryPoint.c_str(), target.c_str(), shaderFlags, 0, &shaderBuffer, &errorMessage);

	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, fileName);
		}
		else
		{
			LPVOID lpMsgBuf;

			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				(DWORD)hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0,
				NULL);

			// Remove terminating newlines
			static_cast<char*>(lpMsgBuf)[strcspn(static_cast<char*>(lpMsgBuf), "\n")] = 0;

			LOG(error) << "Windows Error: " << static_cast<const char*>(lpMsgBuf);

			LocalFree(lpMsgBuf);
		}

		if (shaderBuffer)
		{
			shaderBuffer->Release();
			shaderBuffer = nullptr;
		}

		return false;
	}

	if (!shaderBuffer)
	{
		LOG(error) << "Failed to obtain shader buffer";
		return false;
	}

	if (entryPoint == SHADER_ENTRY_POINT_VERTEX)
	{
		// Create the vertex shader from the buffer.
		hr = device->CreateVertexShader(shaderBuffer->GetBufferPointer(),shaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create Vertex Shader";
			return false;
		}

		// Create the vertex input layout.
		hr = device->CreateInputLayout(inputElementDesc, numElements, shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &m_layout);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create Vertex Input Layout";
			return false;
		}
	} 
	else if (entryPoint == SHADER_ENTRY_POINT_PIXEL)
	{
		// Create the vertex shader from the buffer.
		hr = device->CreatePixelShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create Pixel Shader";
			return false;
		}
	}

	return true;
}

bool dx11::Shader::Initialize(ID3D11Device* device, std::string vsFileName, std::string psFileName)
{
	LOG_FUNC();

	if (device)
	{
		HRESULT hr = E_UNEXPECTED;
		D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
		ZeroMemory(&polygonLayout, sizeof(D3D11_INPUT_ELEMENT_DESC) * 3);
		UINT numElements = 0;
		D3D11_BUFFER_DESC matrixBufferDesc;
		ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		// Create the vertex input layout description.
		// This setup needs to match the Vertex stucture
		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "COLOR";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		polygonLayout[2].SemanticName = "TEXCOORD";
		polygonLayout[2].SemanticIndex = 0;
		polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[2].InputSlot = 0;
		polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[2].InstanceDataStepRate = 0;

		// Get a count of the elements in the layout.
		numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		// Build Vertex Shader
		if (!CompileShader(device, vsFileName, SHADER_TARGET_VERTEX, polygonLayout, numElements))
		{
			LOG(error) << "Failed to obtain Vertex Shader";
			return false;
		}

		// Build Pixel Shader
		if (!CompileShader(device, psFileName, SHADER_TARGET_PIXEL, nullptr, 0))
		{
			LOG(error) << "Failed to obtain Pixel Shader";
			return false;
		}

		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		hr = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create matrix buffer";
			return false;
		}

		// Create a texture sampler state description.
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; // D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; // D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		hr = device->CreateSamplerState(&samplerDesc, &m_sampleState);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create sampler state";
			return false;
		}

		return true;
	}
	else
	{
		LOG(error) << "Bad device.";
	}

	return false;
}


void dx11::Shader::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	SAFE_RELEASE(m_sampleState);

	SAFE_RELEASE(m_matrixBuffer);

	SAFE_RELEASE(m_layout);

	SAFE_RELEASE(m_pixelShader);

	SAFE_RELEASE(m_vertexShader);

	LOG(info) << "Shutdown complete.";
}


void dx11::Shader::OutputShaderErrorMessage(ID3DBlob* errorMessage, std::string shaderFilename)
{
	if (errorMessage)
	{
		char* compileErrors = nullptr;
		unsigned long bufferSize = 0;
		std::string errMsg = "";

		// Get a pointer to the error message text buffer.
		compileErrors = (char*)(errorMessage->GetBufferPointer());

		// Get the length of the message.
		bufferSize = errorMessage->GetBufferSize();

		// Write out the error message.
		for (unsigned int i = 0; i < bufferSize; i++)
		{
			errMsg += compileErrors[i];
		}

		LOG(error) << errMsg;

		// Release the error message.
		errorMessage->Release();
		errorMessage = nullptr;
	}
}


bool dx11::Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11Buffer* constants)
{
	LOG_FUNC();

	if (deviceContext)
	{
		HRESULT hr = E_UNEXPECTED;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		MatrixBufferType* dataPtr = nullptr;
		unsigned int bufferNumber = 0;

		// Transpose the matrices to prepare them for the shader.
		worldMatrix = XMMatrixTranspose(worldMatrix);
		viewMatrix = XMMatrixTranspose(viewMatrix);
		projectionMatrix = XMMatrixTranspose(projectionMatrix);

		// Lock the constant buffer so it can be written to.
		hr = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		if (FAILED(hr))
		{
			LOG(warning) << "Failed to lock matrix buffer";
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);

		// Copy the matrices into the constant buffer.
		dataPtr->world = worldMatrix;
		dataPtr->view = viewMatrix;
		dataPtr->projection = projectionMatrix;

		// Unlock the constant buffer.
		deviceContext->Unmap(m_matrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		bufferNumber = 0;

		// Now set the constant buffer in the vertex shader with the updated values.
		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

		if (shaderResource)
		{
			// Set shader texture resource in the pixel shader.
			deviceContext->PSSetShaderResources(0, 1, &shaderResource);
		}

		// Set the constant buffer.
		if (constants)
		{
			deviceContext->PSSetConstantBuffers(0, 1, &constants);
		}

		return true;
	}
	else
	{
		LOG(error) << "Bad device context.";
	}

	return false;
}


bool dx11::Shader::Render(ID3D11DeviceContext* deviceContext, UINT indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11Buffer* constants)
{
	LOG_FUNC();

	if (deviceContext)
	{
		if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, shaderResource, constants))
		{
			LOG(error) << "Failed to set shader parameters";
			return false;
		}

		UINT viewportCount = 0;
		deviceContext->RSGetViewports(&viewportCount, nullptr);
		if (viewportCount < 1)
		{
			LOG(warning) << std::to_string(viewportCount) << " viewports bound.";

			/*D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			shaderResource->GetDesc(&desc);*/

			// Setup the viewport
			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
			viewport.Width = 1600.0f;
			viewport.Height = 900.0f;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;

			LOG(info) << "Setting viewport.";

			deviceContext->RSSetViewports(1, &viewport);

			viewportCount = 0;
			deviceContext->RSGetViewports(&viewportCount, nullptr);
			LOG(info) << std::to_string(viewportCount) << " viewports bound.";
		}

		// Set the vertex input layout.
		deviceContext->IASetInputLayout(m_layout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		deviceContext->VSSetShader(m_vertexShader, NULL, 0);
		deviceContext->PSSetShader(m_pixelShader, NULL, 0);

		// Set the sampler state in the pixel shader.
		deviceContext->PSSetSamplers(0, 1, &m_sampleState);

		// Draw the triangles.
		deviceContext->DrawIndexed(indexCount, 0, 0);

#ifdef _DEBUG
		DumpD3DDebugMessagesToLog();
#endif

		return true;
	}
	else
	{
		LOG(error) << "Bad device context.";
	}

	return false;
}