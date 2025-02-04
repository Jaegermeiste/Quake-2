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

dx12::Subsystem2D::Subsystem2D()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	m_commandList = nullptr;
	m_2DrenderTargetTexture = nullptr;
	ZeroMemory(&m_2DorthographicMatrix, sizeof(DirectX::XMMATRIX));

}

bool dx12::Subsystem2D::Initialize()
{
	LOG_FUNC();
	
	try {

		D3D12_RESOURCE_DESC renderTargetDesc = {};
		HRESULT hr = E_UNEXPECTED;
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC  rootSignatureDesc = {};
		ComPtr<ID3DBlob> signatureBlob;
		ComPtr<ID3DBlob> errorBlob;

		// Wipe Structs
		ZeroMemory(&renderTargetDesc, sizeof(D3D12_RESOURCE_DESC));
		ZeroMemory(&renderTargetViewDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));

		// Set modified for first run
		ref->cvars->overlayScale->SetModified(true);

		// Set width and height
		m_renderTargetWidth = ref->sys->dx->m_windowWidth;
		m_renderTargetHeight = ref->sys->dx->m_windowHeight;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version supported. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		LOG(info) << "Serializing 2D root signature...";

		hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signatureBlob, &errorBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Failed serializing 2D root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		LOG(info) << "Creating 2D root signature...";

		hr = ref->sys->dx->m_d3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 2D root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		LOG(info) << "Compiling 2D Vertex Shader...";

		if (!m_2DshaderVertex.Compile(ref->cvars->shaderVertex2D->String(), SHADER_TARGET_VERTEX))
		{
			LOG(error) << "Unable to compile 2D Vertex Shader!";
			return false;
		}

		LOG(info) << "Compiling 2D Pixel Shader...";

		if (!m_2DshaderPixel.Compile(ref->cvars->shaderPixel2D->String(), SHADER_TARGET_PIXEL))
		{
			LOG(error) << "Unable to compile 2D Pixel Shader!";
			return false;
		}

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			// Position
			{
				"POSITION",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32A32_FLOAT, // Format
				0,           // InputSlot
				0,           // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Color
			{
				"COLOR",     // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32A32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Texture Coordinate 0
			{
				"TEXCOORD",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			}
		};

		// Find a format that D3D is happy with
		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
		formatSupport.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (SUCCEEDED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
		{
			renderTargetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			LOG(info) << "2D render texture format: DXGI_FORMAT_R32G32B32A32_FLOAT";
		}
		else {
			formatSupport.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			if (SUCCEEDED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
			{
				renderTargetDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				LOG(info) << "2D render texture format: DXGI_FORMAT_R16G16B16A16_FLOAT";
			}
			else {
				formatSupport.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
				if (SUCCEEDED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
				{
					renderTargetDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
					LOG(info) << "2D render texture format: DXGI_FORMAT_R16G16B16A16_UNORM";
				}
				else {
					formatSupport.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
					if (SUCCEEDED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
					{
						renderTargetDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
						LOG(info) << "2D render texture format: DXGI_FORMAT_R10G10B10A2_UNORM";
					}
					else
					{
						formatSupport.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
						if (SUCCEEDED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
						{
							renderTargetDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
							LOG(info) << "2D render texture format: DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
						}
						else
						{
							renderTargetDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
							LOG(info) << "2D render texture format: DXGI_FORMAT_B8G8R8A8_UNORM";
						}
					}
				}
			}
		}

		D3D12_BLEND_DESC blendStateDescription;
		ZeroMemory(&blendStateDescription, sizeof(D3D12_BLEND_DESC));
		for (int i = 0; i < 8; i++)
		{
			// Create an alpha enabled blend state description.
			blendStateDescription.RenderTarget[i].BlendEnable = TRUE;
			blendStateDescription.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendStateDescription.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendStateDescription.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
			blendStateDescription.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendStateDescription.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendStateDescription.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendStateDescription.RenderTarget[i].RenderTargetWriteMask = 0x0f;
		}
		blendStateDescription.IndependentBlendEnable = TRUE;

		D3D12_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;  // Solid fill (no wireframe)
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;   // No culling (both sides visible)
		rasterizerDesc.FrontCounterClockwise = FALSE;     // Default front-face winding order
		rasterizerDesc.DepthBias = 0;                     // No depth bias
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthClipEnable = FALSE;           // No depth clipping for 2D
		rasterizerDesc.MultisampleEnable = FALSE;         // No MSAA (handled in textures)
		rasterizerDesc.AntialiasedLineEnable = FALSE;     // Not needed for 2D
		rasterizerDesc.ForcedSampleCount = 0;             // Default sample count
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;  // Default rasterization

		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = FALSE;  // Disable depth testing
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // No depth writes
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;  // Always pass
		depthStencilDesc.StencilEnable = FALSE;  // Disable stencil

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_2DshaderVertex.Blob().Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_2DshaderPixel.Blob().Get());
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = blendStateDescription;
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = renderTargetDesc.Format;
		psoDesc.SampleDesc.Count = 1;

		LOG(info) << "Creating 2D Graphics Pipeline State Object...";

		hr = ref->sys->dx->m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 2D Graphics Pipeline State Object: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		// Setup the render target texture description.
		renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		renderTargetDesc.Alignment = 0;
		renderTargetDesc.Width = m_renderTargetWidth;
		renderTargetDesc.Height = m_renderTargetHeight;
		renderTargetDesc.MipLevels = 1;
		renderTargetDesc.DepthOrArraySize = 1;
		renderTargetDesc.SampleDesc.Count = 1;
		renderTargetDesc.SampleDesc.Quality = 0;
		renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = renderTargetDesc.Format;
#ifndef _DEBUG
		//DirectX::Colors::Transparent
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 0.0f;
#else
		clearValue.Color[0] = 1.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 0.25f;
#endif

		// Create the render target texture.
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

		m_2drenderTargetState = D3D12_RESOURCE_STATE_PRESENT;
		hr = ref->sys->dx->m_d3dDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&renderTargetDesc,
			m_2drenderTargetState,
			&clearValue,
			IID_PPV_ARGS(&m_2DrenderTargetTexture));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 2D Render Target Texture: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_2DrenderTargetTexture)
		{
			m_2DrenderTargetTexture->SetName(L"2D RenderTarget");
		}

		// Setup the description of the render target view.
		renderTargetViewDesc.Format = renderTargetDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		m_2dRTVHandle = ref->sys->dx->m_descriptorHeapRTV->Allocate();

		// Create the render target view.
		ref->sys->dx->m_d3dDevice->CreateRenderTargetView(m_2DrenderTargetTexture.Get(), &renderTargetViewDesc, ref->sys->dx->m_descriptorHeapRTV->GetCPUDescriptorHandle(m_2dRTVHandle));
		
		m_2dSRVHandle = ref->sys->dx->m_descriptorHeapCBVSRVUAV->Allocate();

		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = renderTargetDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		// Create the shader resource view.
		ref->sys->dx->m_d3dDevice->CreateShaderResourceView(m_2DrenderTargetTexture.Get(), &shaderResourceViewDesc, ref->sys->dx->m_descriptorHeapCBVSRVUAV->GetCPUDescriptorHandle(m_2dSRVHandle));

		// Setup the viewport
		ZeroMemory(&m_viewport, sizeof(D3D12_VIEWPORT));
		m_viewport.Width = (FLOAT)m_renderTargetWidth;
		m_viewport.Height = (FLOAT)m_renderTargetHeight;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;

		LOG(info) << "Setting viewport.";

		// Setup the scissor rect
		ZeroMemory(&m_scissorRect, sizeof(D3D12_RECT));
		m_scissorRect.left = 0.0f;
		m_scissorRect.top = 0.0f;
		m_scissorRect.right = (FLOAT)m_renderTargetWidth;
		m_scissorRect.bottom = (FLOAT)m_renderTargetHeight;

		LOG(info) << "Setting scissor rect.";

		// Create an orthographic projection matrix for 2D rendering.
		m_2DorthographicMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(m_renderTargetWidth), static_cast<float>(m_renderTargetHeight), 0.0f, ref->cvars->zNear2D->Float(), ref->cvars->zFar2D->Float());

		// Create the constants buffer
		auto constantBuffer = ref->res->CreateResource<ConstantBuffer2D>(L"ConstantBuffer2D");

		if (constantBuffer)
		{
			m_constantBufferHandle = constantBuffer->GetHandle();

			ShaderConstants2D constants = {};

			// Set values
			XMStoreFloat4x4(&constants.world, XMMatrixTranspose(XMMatrixIdentity()));
			XMStoreFloat4x4(&constants.view, XMMatrixTranspose(XMMatrixIdentity()));
			XMStoreFloat4x4(&constants.projection, XMMatrixTranspose(m_2DorthographicMatrix));
			constants.brightness = ref->cvars->overlayBrightness->Float();
			constants.contrast = ref->cvars->overlayContrast->Float();

			// Create the constants buffer.
			constantBuffer->CreateConstantBuffer(&constants, sizeof(constants));

			// Clear the modified flag
			ref->cvars->overlayBrightness->SetModified(false);
			ref->cvars->overlayContrast->SetModified(false);
		}

		m_commandList = std::make_shared<CommandList>(L"2D", m_rootSignature, m_pipelineState, m_viewport, m_scissorRect, m_constantBufferHandle, m_2dRTVHandle);
		
		// Calculate the width of the overlay.
		int width = static_cast<int>(static_cast<float>(m_renderTargetWidth) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the left side of the overlay.
		int x = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowWidth) - width) / 2);

		// Calculate the height of the overlay.
		int height = static_cast<int>(static_cast<float>(m_renderTargetHeight) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the top of the overlay.
		int y = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowHeight) - height) / 2);
		
		if (!m_renderTargetQuad.Initialize(x, y, width, height, DirectX::Colors::White))
		{
			LOG(error) << "Failed to properly initialize render target quad.";
			return false;
		}

		if (!m_generalPurposeQuad.Initialize(x, y, width, height, DirectX::Colors::White))
		{
			LOG(error) << "Failed to properly initialize general purpose quad.";
			return false;
		}

		if (!m_fadeScreenQuad.Initialize(0, 0, msl::utilities::SafeInt<int>(ref->sys->dx->m_windowWidth), msl::utilities::SafeInt<int>(ref->sys->dx->m_windowHeight), { 0.0f, 0.0f, 0.0f, 0.75f }))
		{
			LOG(error) << "Failed to properly initialize fade screen quad.";
			return false;
		}

		ref->draw->Initialize();
		
		if (m_commandList)
		{
			if (m_2drenderTargetState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				auto renderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_2DrenderTargetTexture.Get(),
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_commandList->List()->ResourceBarrier(1, &renderTargetBarrier);
				m_2drenderTargetState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}

			m_commandList->Prepare();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	LOG(info) << "Successfully initialized 2D subsystem.";

	return true;
}

void dx12::Subsystem2D::Clear()
{
	LOG_FUNC();
	
	// Clear 2D
	if (m_commandList && m_commandList->IsOpen())
	{
		LOG(trace) << "Clearing overlay RenderTargetView.";

		// Clear the GUI Overlay buffer to transparent
#if defined(DEBUG) || defined (_DEBUG)
		DirectX::XMVECTORF32 ClearColor = { 1.000000000f, 0.000000000f, 0.000000000f, 0.250000000f };
		m_commandList->List()->ClearRenderTargetView(ref->sys->dx->m_descriptorHeapRTV->GetCPUDescriptorHandle(m_2dRTVHandle), ClearColor, 0, nullptr);
#else
		m_commandList->List()->ClearRenderTargetView(ref->sys->dx->m_descriptorHeapRTV->GetCPUDescriptorHandle(m_2dRTVHandle), DirectX::Colors::Transparent, 0, nullptr);
#endif
	}
	
}

void dx12::Subsystem2D::Update()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	
	if (m_commandList && m_commandList->IsOpen())
	{
		// Update the constants buffer as necessary
		if (ref->cvars->overlayBrightness->Modified() || ref->cvars->overlayContrast->Modified())
		{
			auto constantBuffer = ref->res->GetResource<ConstantBuffer2D>(m_constantBufferHandle);

			if (constantBuffer)
			{
				ShaderConstants2D constants = {};

				// Set values
				XMStoreFloat4x4(&constants.world, XMMatrixTranspose(XMMatrixIdentity()));
				XMStoreFloat4x4(&constants.view, XMMatrixTranspose(XMMatrixIdentity()));
				XMStoreFloat4x4(&constants.projection, XMMatrixTranspose(m_2DorthographicMatrix));
				constants.brightness = ref->cvars->overlayBrightness->Float();
				constants.contrast = ref->cvars->overlayContrast->Float();

				// Update the constants buffer.
				constantBuffer->UpdateConstantBuffer(m_commandList, &constants, sizeof(constants));

				m_commandList->List()->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());

				// Clear the modified flag
				ref->cvars->overlayBrightness->SetModified(false);
				ref->cvars->overlayContrast->SetModified(false);
			}
		}
	}
}

void dx12::Subsystem2D::Render()
{
	LOG_FUNC();
	
	HRESULT hr = E_UNEXPECTED;

	if (ref->sys->dx->m_commandListSwap && ref->sys->dx->m_commandListSwap->IsOpen())
	{
		// Calculate the width of the overlay.
		int width = static_cast<int>(static_cast<float>(m_renderTargetWidth) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the left side of the overlay.
		int x = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowWidth) - width) / 2);

		// Calculate the height of the overlay.
		int height = static_cast<int>(static_cast<float>(m_renderTargetHeight) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the top of the overlay.
		int y = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowHeight) - height) / 2);

		// Render the 2D Overlay to the Render Target
		if (m_commandList && m_commandList->IsOpen())
		{
			m_commandList->Close();
			m_commandList->Execute();
			
			// Draw the 2D render target to the back buffer
			auto oldBackBufferState = ref->sys->dx->m_backBufferRenderTargetStates[ref->sys->dx->m_backBufferIndex];
			if (oldBackBufferState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				// Indicate that the back buffer will now be used as a render target.
				auto backBufferBarrierIn = CD3DX12_RESOURCE_BARRIER::Transition(ref->sys->dx->m_backBufferRenderTargets[ref->sys->dx->m_backBufferIndex].Get(),
					oldBackBufferState, D3D12_RESOURCE_STATE_RENDER_TARGET);
				ref->sys->dx->m_commandListSwap->List()->ResourceBarrier(1, &backBufferBarrierIn);
			}

			auto oldRenderTargetState = m_2drenderTargetState;
			if (oldRenderTargetState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			{
				// Indicate that the render target will now be used as a source texture.
				auto renderTargetBarrierIn = CD3DX12_RESOURCE_BARRIER::Transition(m_2DrenderTargetTexture.Get(),
					oldRenderTargetState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				ref->sys->dx->m_commandListSwap->List()->ResourceBarrier(1, &renderTargetBarrierIn);
				m_2drenderTargetState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}

			// Bind the SRV
			ref->sys->dx->m_commandListSwap->List()->SetGraphicsRootDescriptorTable(1, ref->sys->dx->m_descriptorHeapCBVSRVUAV->GetGPUDescriptorHandle(m_2dSRVHandle));
			
			m_renderTargetQuad.Render(ref->sys->dx->m_commandListSwap, x, y, width, height, DirectX::Colors::White);

			if (m_2drenderTargetState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				// Put it back to the old state.
				auto renderTargetBarrierOut = CD3DX12_RESOURCE_BARRIER::Transition(m_2DrenderTargetTexture.Get(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, oldRenderTargetState);
				ref->sys->dx->m_commandListSwap->List()->ResourceBarrier(1, &renderTargetBarrierOut);
				m_2drenderTargetState = oldRenderTargetState;
			}

			if (oldBackBufferState != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				// Put it back to the old state.
				auto backBufferBarrierOut = CD3DX12_RESOURCE_BARRIER::Transition(ref->sys->dx->m_backBufferRenderTargets[ref->sys->dx->m_backBufferIndex].Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, oldBackBufferState);
				ref->sys->dx->m_commandListSwap->List()->ResourceBarrier(1, &backBufferBarrierOut);
			}

			m_commandList->Prepare();
		}
	}

#ifdef _DEBUG
	DumpD3DDebugMessagesToLog();
#endif
	
}

void dx12::Subsystem2D::FadeScreen()
{
	LOG_FUNC();

	std::shared_ptr<dx12::Texture2D> image = ref->media->img->Load(WHITE_TEXTURE_NAME, it_pic);

	if (m_commandList && m_commandList->IsOpen())
	{
		if (image)
		{
			image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_fadeScreenQuad.Render(m_commandList);
		}
	}
}

void dx12::Subsystem2D::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	if (m_commandList)
	{
		m_commandList->Close();
	}

	ref->sys->dx->WaitForGPU();

	SAFE_RELEASE(m_commandList);

	SAFE_RELEASE(m_pipelineState);

	SAFE_RELEASE(m_rootSignature);

	m_2DshaderVertex.Shutdown();

	m_2DshaderPixel.Shutdown();

	m_generalPurposeQuad.Shutdown();

	m_renderTargetQuad.Shutdown();

	m_fadeScreenQuad.Shutdown();

	ref->draw->Shutdown();

	SAFE_RELEASE(m_2DrenderTargetTexture);

	LOG(info) << "Shutdown complete.";
}
