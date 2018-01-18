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
	BOOST_LOG_NAMED_SCOPE("Shader");

	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_sampleState = nullptr;
}

bool dx11::Shader::CompileShader(ID3D11Device* device, std::string fileName, shaderTarget target, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements)
{
	BOOST_LOG_NAMED_SCOPE("Shader::CompileShader");

	ID3DBlob* shaderBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;
	std::string entryPoint = "";
	HRESULT hr;

	// We need this to get a compliant path string
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertToWide;

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
		target += "_6_0";
		break;
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
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
	default:
		if ((target != SHADER_TARGET_VERTEX) && (target != SHADER_TARGET_PIXEL))
		{
			LOG(info) << "Shader File " << fileName << " with profile " << target << "_3_0 is not supported in D3D_FEATURE_LEVEL_9_x";
			return false;
		}
		target += "_3_0";
	}
	
	// Compile the shader code.
	LOG(info) << "Compiling Shader File " << fileName << " with target " << target << " and entry point " << entryPoint;

	hr = D3DCompileFromFile(convertToWide.from_bytes(fileName).c_str(), NULL, NULL, entryPoint.c_str(), target.c_str(), D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, 0, &shaderBuffer, &errorMessage);
	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, fileName);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			LOG(error) << "Missing Shader File " << fileName;
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
	BOOST_LOG_NAMED_SCOPE("Shader::Initialize");

	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	ZeroMemory(&polygonLayout, sizeof(D3D11_INPUT_ELEMENT_DESC) * 2);
	UINT numElements = 0;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Create the vertex input layout description.
	// This setup needs to match the Vertex stucture
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

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
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
		return false;
	}

	return true;
}


void dx11::Shader::Shutdown()
{
	BOOST_LOG_NAMED_SCOPE("Shader::Shutdown");

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


bool dx11::Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr = nullptr;
	unsigned int bufferNumber = 0;

	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

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
	dataPtr = (MatrixBufferType*)mappedResource.pData;

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

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


bool dx11::Shader::Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture))
	{
		return false;
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

	return true;
}