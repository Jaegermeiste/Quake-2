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

DXGI_FORMAT dx12::RenderTarget::FindFormat()
{
	LOG_FUNC();

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

	try
	{
		// Find a format that D3D is happy with
		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
		formatSupport.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (SUCCEEDED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
		{
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			LOG(info) << "Render texture format: DXGI_FORMAT_R32G32B32A32_FLOAT";
		}
		else {
			formatSupport.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			if (SUCCEEDED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
			{
				format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				LOG(info) << "Render texture format: DXGI_FORMAT_R16G16B16A16_FLOAT";
			}
			else {
				formatSupport.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
				if (SUCCEEDED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
				{
					format = DXGI_FORMAT_R16G16B16A16_UNORM;
					LOG(info) << "Render texture format: DXGI_FORMAT_R16G16B16A16_UNORM";
				}
				else {
					formatSupport.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
					if (SUCCEEDED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
					{
						format = DXGI_FORMAT_R10G10B10A2_UNORM;
						LOG(info) << "Render texture format: DXGI_FORMAT_R10G10B10A2_UNORM";
					}
					else
					{
						formatSupport.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
						if (SUCCEEDED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
						{
							format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
							LOG(info) << "Render texture format: DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
						}
						else
						{
							format = DXGI_FORMAT_B8G8R8A8_UNORM;
							LOG(info) << "Render texture format: DXGI_FORMAT_B8G8R8A8_UNORM";
						}
					}
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

	return format;
}

bool dx12::RenderTarget::CreateRenderTarget(size_t width, size_t height, DirectX::XMVECTORF32 clearColor = DirectX::Colors::Transparent)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		if ((width > 0) && (height > 0))
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};

			// Wipe Structs
			ZeroMemory(&m_resourceDesc, sizeof(D3D12_RESOURCE_DESC1));
			ZeroMemory(&renderTargetViewDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
			ZeroMemory(&shaderResourceViewDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));

			// Setup the render target texture description.
			m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			m_resourceDesc.Alignment = 0;
			m_resourceDesc.Width = width;
            m_resourceDesc.Height = msl::utilities::SafeInt<UINT>(height);
			m_resourceDesc.Format = FindFormat();
			m_resourceDesc.MipLevels = 1;
			m_resourceDesc.DepthOrArraySize = 1;
			m_resourceDesc.SampleDesc.Count = 1;
			m_resourceDesc.SampleDesc.Quality = 0;
			m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			m_resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			// Set up optimized clear
			D3D12_CLEAR_VALUE clearValue = {};
			clearValue.Format = m_resourceDesc.Format;
			clearValue.Color[0] = clearColor[0];
			clearValue.Color[1] = clearColor[1];
			clearValue.Color[2] = clearColor[2];
			clearValue.Color[3] = clearColor[3];

			m_clearColor = clearColor;

			// Create the render target texture.
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

			m_state = D3D12_RESOURCE_STATE_PRESENT;
			hr = ref->sys->dx->Device()->CreateCommittedResource2(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&m_resourceDesc,
				m_state,
				&clearValue,
				nullptr,
				IID_PPV_ARGS(&m_resource));

			if (FAILED(hr))
			{
				LOG(error) << "Unable to create Render Target: " << GetD3D12ErrorMessage(hr);
				return false;
			}

			if (m_resource)
			{
				m_resource->SetName(m_name.c_str());

				// Setup the description of the render target view.
				renderTargetViewDesc.Format = m_resourceDesc.Format;
				renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				renderTargetViewDesc.Texture2D.MipSlice = 0;

				m_RTVhandle = ref->sys->dx->HeapRTV()->Allocate();

				// Create the render target view.
				ref->sys->dx->Device()->CreateRenderTargetView(m_resource.Get(), &renderTargetViewDesc, ref->sys->dx->HeapRTV()->GetCPUDescriptorHandle(m_RTVhandle));

				m_SRVhandle = ref->sys->dx->HeapCBVSRVUAV()->Allocate();

				// Setup the description of the shader resource view.
				shaderResourceViewDesc.Format = m_resourceDesc.Format;
				shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
				shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				// Create the shader resource view.
				ref->sys->dx->Device()->CreateShaderResourceView(m_resource.Get(), &shaderResourceViewDesc, ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_SRVhandle));

				// Initialize the Quad
				// Calculate the width of the quad.
				int quadWidth = static_cast<int>(static_cast<float>(width) * m_quadScale);

				// Calculate the screen coordinates of the left side of the quad.
				int x = ((msl::utilities::SafeInt<int>(width) - quadWidth) / 2);

				// Calculate the height of the quad.
				int quadHeight = static_cast<int>(static_cast<float>(height) * m_quadScale);

				// Calculate the screen coordinates of the top of the quad.
				int y = ((msl::utilities::SafeInt<int>(height) - quadHeight) / 2);

				if (!m_quad.Initialize(x, y, quadWidth, quadHeight, DirectX::Colors::White))
				{
					LOG(error) << "Failed to properly initialize render target quad.";
					return false;
				}
			}

			return true;
		}
		else
		{
			LOG(error) << "Invalid width (" << width << ") or height (" << height << ")!";
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

void dx12::RenderTarget::Clear(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		if (commandList && commandList->IsOpen())
		{
			LOG(trace) << "Clearing RenderTargetView.";

			// Clear the GUI Overlay buffer to transparent
			commandList->List()->ClearRenderTargetView(GetRTVCPUDescriptorHandle(), m_clearColor, 0, nullptr);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::RenderTarget::RenderQuad(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		// Calculate the width of the quad.
		int quadWidth = msl::utilities::SafeInt<int>(m_resourceDesc.Width); //static_cast<int>(static_cast<float>(m_resourceDesc.Width) * m_quadScale);

		// Calculate the virtual screen coordinates of the left side of the quad.
		//int vx = ((msl::utilities::SafeInt<int>(m_resourceDesc.Width) - quadWidth) / 2);

		// Calculate the height of the quad.
		int quadHeight = msl::utilities::SafeInt<int>(m_resourceDesc.Height); //static_cast<int>(static_cast<float>(m_resourceDesc.Height) * m_quadScale);

		// Calculate the virtual screen coordinates of the top of the quad.
		//int vy = ((msl::utilities::SafeInt<int>(m_resourceDesc.Height) - quadHeight) / 2);

		float aspectRatioQuad = static_cast<float>(quadWidth) / quadHeight;

		// Scale in an aspect-ratio correct manner to fill the backbuffer
		int windowWidth = msl::utilities::SafeInt<int>(ref->sys->dx->WindowWidth());
		int windowHeight = msl::utilities::SafeInt<int>(ref->sys->dx->WindowHeight());

		float aspectRatioWindow = static_cast<float>(windowWidth) / windowHeight;

		int width = 0;
		int height = 0;

		if (aspectRatioQuad > aspectRatioWindow) {
			width = windowWidth;
			height = windowWidth / aspectRatioQuad;
		}
		else 
		{
			height = windowHeight;
			width = windowHeight * aspectRatioQuad;
		}

		width = static_cast<int>(static_cast<float>(width) * m_quadScale);
		height = static_cast<int>(static_cast<float>(height) * m_quadScale);

		int x = (windowWidth - width) / 2;
		int y = (windowHeight - height) / 2;

		m_quad.Render(commandList, x, y, width, height, DirectX::Colors::White);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::RenderTarget::Transition(std::shared_ptr<CommandList> commandList, D3D12_RESOURCE_STATES destinationState)
{
	LOG_FUNC();

	try
	{
		if (m_state != destinationState)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
				m_state, destinationState);
			commandList->List()->ResourceBarrier(1, &barrier);
			m_state = destinationState;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE dx12::RenderTarget::GetRTVCPUDescriptorHandle() const { return ref->sys->dx->HeapRTV()->GetCPUDescriptorHandle(m_RTVhandle); }

D3D12_GPU_DESCRIPTOR_HANDLE dx12::RenderTarget::GetRTVGPUDescriptorHandle() const { return ref->sys->dx->HeapRTV()->GetGPUDescriptorHandle(m_RTVhandle); }

D3D12_CPU_DESCRIPTOR_HANDLE dx12::RenderTarget::GetSRVCPUDescriptorHandle() const { return ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_SRVhandle); }

D3D12_GPU_DESCRIPTOR_HANDLE dx12::RenderTarget::GetSRVGPUDescriptorHandle() const { return ref->sys->dx->HeapCBVSRVUAV()->GetGPUDescriptorHandle(m_SRVhandle); }
