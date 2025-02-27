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

	try
	{
		m_commandList = nullptr;
		m_renderTarget = nullptr;
		ZeroMemory(&m_2DorthographicMatrix, sizeof(DirectX::XMMATRIX));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

bool dx12::Subsystem2D::Initialize()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	
	try {
		// Set modified for first run
		ref->cvars->overlayScale->SetModified(true);

		if (!CreateRootSignatures())
		{
			LOG(error) << "Failed to create root signatures.";
			return false;
		}

		if (!CompileShaders())
		{
			LOG(error) << "Failed to compile shaders.";
			return false;
		}

		if (!CreateRenderTarget())
		{
			LOG(error) << "Failed to create render target.";
			return false;
		}

		if (!CreateConstantsBuffer())
		{
			LOG(error) << "Failed to create constants buffer.";
			return false;
		}

		if (!CreateGraphicsPipelineStateObject())
		{
			LOG(error) << "Failed to create graphics pipeline state object.";
			return false;
		}
				
		// Setup the viewport
		ZeroMemory(&m_viewport, sizeof(D3D12_VIEWPORT));
		m_viewport.Width = (FLOAT)m_renderTarget->Width();
		m_viewport.Height = (FLOAT)m_renderTarget->Height();
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;

		LOG(info) << "Setting viewport.";

		// Setup the scissor rect
		ZeroMemory(&m_scissorRect, sizeof(D3D12_RECT));
		m_scissorRect.left = 0.0f;
		m_scissorRect.top = 0.0f;
		m_scissorRect.right = (FLOAT)m_renderTarget->Width();
		m_scissorRect.bottom = (FLOAT)m_renderTarget->Height();

		LOG(info) << "Setting scissor rect.";

		m_commandList = std::make_shared<CommandList>(L"2dCommandList", m_rootSignature, m_pipelineState, m_viewport, m_scissorRect, m_constantBufferHandle, m_renderTarget->GetRTVHandle());

		if (!m_fadeScreenQuad.Initialize(0, 0, msl::utilities::SafeInt<int>(ref->sys->dx->m_modeWidth), msl::utilities::SafeInt<int>(ref->sys->dx->m_modeHeight), { 0.0f, 0.0f, 0.0f, 0.75f }))
		{
			LOG(error) << "Failed to properly initialize fade screen quad.";
			return false;
		}

		ref->draw->Initialize();
		
		if (m_commandList)
		{
			if (m_renderTarget)
			{
				m_renderTarget->Transition(m_commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
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

bool dx12::Subsystem2D::CreateRootSignatures()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC  rootSignatureDesc = {};
		ComPtr<ID3DBlob> signatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version supported. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(ref->sys->dx->m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
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

		m_rootSignature->SetName(L"2D Root Signature");

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

bool dx12::Subsystem2D::CompileShaders()
{
	LOG_FUNC();

	try
	{
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

bool dx12::Subsystem2D::CreateGraphicsPipelineStateObject()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		if (m_renderTarget)
		{
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
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_2DshaderVertex.D3DBlob().Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_2DshaderPixel.D3DBlob().Get());
			psoDesc.RasterizerState = rasterizerDesc;
			psoDesc.BlendState = blendStateDescription;
			psoDesc.DepthStencilState = depthStencilDesc;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = m_renderTarget->Format();
			psoDesc.SampleDesc.Count = 1;

			LOG(info) << "Creating 2D Graphics Pipeline State Object...";

			hr = ref->sys->dx->m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

			if (FAILED(hr))
			{
				LOG(error) << "Unable to create 2D Graphics Pipeline State Object: " << GetD3D12ErrorMessage(hr);
				return false;
			}

			return true;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::Subsystem2D::CreateConstantsBuffer()
{
	LOG_FUNC();

	try
	{
		// Create an orthographic projection matrix for 2D rendering.
		m_2DorthographicMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(m_renderTarget->Width()), static_cast<float>(m_renderTarget->Height()), 0.0f, ref->cvars->zNear2D->Float(), ref->cvars->zFar2D->Float());
		
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

bool dx12::Subsystem2D::CreateRenderTarget()
{
	LOG_FUNC();

	try
	{
		if (m_renderTarget)
		{
			SAFE_RELEASE(m_renderTarget);
		}

		m_renderTarget = ref->res->GetOrCreateResource<RenderTarget>(L"2dRenderTarget");

		if (!m_renderTarget)
		{
			LOG(error) << "Failed to obtain 2d Render Target resource!";

			return false;
		}

		if (!m_renderTarget->CreateRenderTarget(ref->sys->dx->m_modeWidth, ref->sys->dx->m_modeHeight, DirectX::Colors::Transparent))
		{
			LOG(error) << "Failed to create 2d Render Target!";

			return false;
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

void dx12::Subsystem2D::Clear()
{
	LOG_FUNC();
	
	try
	{
		if (m_commandList && m_commandList->IsOpen() && m_renderTarget)
		{
			LOG(trace) << "Clearing overlay RenderTargetView.";

			m_renderTarget->Clear(m_commandList);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem2D::Update()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	
	try
	{
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
				else
				{
					LOG(warning) << "Failed to get ConstantBuffer2D resource for handle: " << m_constantBufferHandle;
				}
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

void dx12::Subsystem2D::Render(std::shared_ptr<CommandList> commandListSwap)
{
	LOG_FUNC();
	
	HRESULT hr = E_UNEXPECTED;

	try
	{
		if (commandListSwap && commandListSwap->IsOpen())
		{
			// Render the 2D Overlay to the Render Target
			if (m_commandList && m_commandList->IsOpen())
			{
				m_commandList->Close();
				m_commandList->Execute();

				if (m_renderTarget)
				{
					m_renderTarget->Transition(commandListSwap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

					// Bind the SRV
					commandListSwap->List()->SetGraphicsRootDescriptorTable(1, m_renderTarget->GetSRVGPUDescriptorHandle());

					if (ref->cvars->overlayScale->Modified())
					{
						m_renderTarget->SetQuadScale(ref->cvars->overlayScale->Float());
						ref->cvars->overlayScale->SetModified(false);
					}

					// Render the render target to the backbuffer
					m_renderTarget->RenderQuad(commandListSwap);

					m_renderTarget->Transition(commandListSwap, D3D12_RESOURCE_STATE_RENDER_TARGET);
				}

				m_commandList->Prepare();
			}
		}

#ifdef _DEBUG
		DumpD3DDebugMessagesToLog();
#endif
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem2D::FadeScreen()
{
	LOG_FUNC();

	try
	{
		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(WHITE_TEXTURE_NAME, it_pic);

		if (m_commandList && m_commandList->IsOpen())
		{
			if (image)
			{
				image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

				m_fadeScreenQuad.Render(m_commandList);
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

void dx12::Subsystem2D::Shutdown()
{
	LOG_FUNC();

	try
	{
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

		m_fadeScreenQuad.Shutdown();

		ref->draw->Shutdown();

		SAFE_RELEASE(m_renderTarget);

		LOG(info) << "Shutdown complete.";
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
