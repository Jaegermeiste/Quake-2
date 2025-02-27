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

#include "dx12_local.hpp"

dx12::DescriptorHeap::DescriptorHeap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
	: m_device(device), m_type(type), m_descriptorCount(numDescriptors), m_shaderVisible(shaderVisible)
{
    try
    {
		assert(device != nullptr);

		CreateHeap(type, numDescriptors, shaderVisible);
		m_handles.reserve(numDescriptors);
    }
    catch (const std::runtime_error& e) {
        LOG(error) << "Runtime Error: " << e.what();
    }
    catch (const std::exception& e) {
        LOG(error) << "General Exception: " << e.what();
    }
}

void dx12::DescriptorHeap::CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
{
	try
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));

		if (SUCCEEDED(hr))
		{
			LOG(info) << "Successfully created " << m_descriptorTypes.at(type) << ".";
		}
		else {
			LOG(error) << "Unable to create " << m_descriptorTypes.at(type) << ".";
		}

		m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(type);
		m_heapStartCPU = m_heap->GetCPUDescriptorHandleForHeapStart();
		if (shaderVisible)
		{
			m_heapStartGPU = m_heap->GetGPUDescriptorHandleForHeapStart();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::DescriptorHeap::ResizeHeap(UINT newSize)
{
	try
	{
		if (newSize <= m_descriptorCount)
			return;

		// Keep track of the old heap and its handles before creating a new one
		ComPtr<ID3D12DescriptorHeap> oldHeap = m_heap;
		D3D12_CPU_DESCRIPTOR_HANDLE oldHeapStartCPU = oldHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE oldHeapStartGPU = {};

		if (m_shaderVisible)
		{
			oldHeapStartGPU = oldHeap->GetGPUDescriptorHandleForHeapStart();
		}

		// Create a new heap with the updated size
		CreateHeap(m_type, newSize, m_shaderVisible);

		m_device->CopyDescriptorsSimple(static_cast<UINT>(GetAllocatedCount()), m_heapStartCPU, oldHeapStartCPU, m_type);

		// Copy descriptors from the old heap to the new heap
		for (UINT i = 0; i < GetAllocatedCount(); ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE srcCPU = { oldHeapStartCPU.ptr + i * m_descriptorSize };
			D3D12_CPU_DESCRIPTOR_HANDLE dstCPU = { m_heapStartCPU.ptr + i * m_descriptorSize };

			m_handles[i].first.ptr = dstCPU.ptr; // Update stored handle

			if (m_shaderVisible)
			{
				D3D12_GPU_DESCRIPTOR_HANDLE srcGPU = { oldHeapStartGPU.ptr + i * m_descriptorSize };
				D3D12_GPU_DESCRIPTOR_HANDLE dstGPU = { m_heapStartGPU.ptr + i * m_descriptorSize };

				m_handles[i].second->ptr = dstGPU.ptr; // Update stored handle
			}
		}

		m_descriptorCount = newSize;

		SAFE_RELEASE(oldHeap);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}


dxhandle_t dx12::DescriptorHeap::Allocate()
{
	try
	{
		if (GetAllocatedCount() >= m_descriptorCount)
		{
			ResizeHeap(m_descriptorCount * 2);
		}

		unsigned long long index = GetAllocatedCount();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
		cpuHandle.ptr = m_heapStartCPU.ptr + index * m_descriptorSize;

		std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> gpuHandle = std::nullopt;
		if (m_shaderVisible)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandlePtr = {};
			gpuHandlePtr.ptr = m_heapStartGPU.ptr + index * m_descriptorSize;
			gpuHandle = gpuHandlePtr;
		}

		m_handles.push_back({ cpuHandle, gpuHandle ? *gpuHandle : std::optional<D3D12_GPU_DESCRIPTOR_HANDLE>(std::nullopt) });

		return GetAllocatedCount() - 1;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return 0;
}

dx12::DescriptorHeap::~DescriptorHeap()
{
    LOG_FUNC();

	try
	{
		m_device = nullptr;
		SAFE_RELEASE(m_heap);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
