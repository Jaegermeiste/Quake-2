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
#include "dx12_locaL.hpp"

dx12::RaytracingBuffer::RaytracingBuffer(std::wstring name) : Resource(name)
{
	m_type = RESOURCE_RAYTRACINGBUFFER;
	m_count = 0;
}

bool dx12::RaytracingBuffer::CreateBuffer(size_t width, size_t height) 
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		m_width = width;
		m_height = height;

		m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		m_resourceDesc.Alignment = 0;
		m_resourceDesc.Width = width * height * 4; // Size of the buffer (width * height * 4 bytes per pixel)
		m_resourceDesc.Height = 1;
		m_resourceDesc.DepthOrArraySize = 1;
		m_resourceDesc.MipLevels = 1;
		m_resourceDesc.Format = DXGI_FORMAT_UNKNOWN; //DXGI_FORMAT_R32G32B32A32_FLOAT; // Format of the buffer
		m_resourceDesc.SampleDesc.Count = 1;
		m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		m_resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		auto heapProps =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		hr = ref->sys->dx->Device()->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&m_resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_resource));

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create buffer resource: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_resource)
		{
			m_resource->SetName(m_name.c_str());
		}

		m_count = 1;

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

void dx12::RaytracingBuffer::CreateUAV()
{
	LOG_FUNC();

	try
	{
		// Get the descriptor heap handle and create the UAV
		m_uavHandle = ref->sys->dx->HeapCBVSRVUAV()->Allocate();
		m_cachedD3D12UAVHandle = ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_uavHandle);
		ref->sys->dx->Device()->CreateUnorderedAccessView(m_resource.Get(), nullptr, nullptr, m_cachedD3D12UAVHandle);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::RaytracingBuffer::RefreshUAV()
{
	LOG_FUNC();

	try
	{
		if (m_uavHandle == 0)
		{
			CreateUAV();
		}
		else if (ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_uavHandle).ptr != m_cachedD3D12UAVHandle.ptr)
		{
			// Get the descriptor heap handle and create a fresh SRV
			m_cachedD3D12UAVHandle = ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_uavHandle);
			ref->sys->dx->Device()->CreateUnorderedAccessView(m_resource.Get(), nullptr, nullptr, m_cachedD3D12UAVHandle);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::RaytracingBuffer::BindUAV(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		if (commandList && commandList->IsOpen())
		{
			RefreshUAV();

			commandList->List()->SetDescriptorHeaps(1, &(ref->sys->dx->HeapCBVSRVUAV()->GetHeap()));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

