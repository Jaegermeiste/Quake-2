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

#ifndef __DX12_DESCRIPTORHEAP_HPP__
#define __DX12_DESCRIPTORHEAP_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class DescriptorHeap
	{
	private:
		const std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, std::wstring> m_descriptorTypes = {
		{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, L"D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV" },
		{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, L"D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER" },
		{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, L"D3D12_DESCRIPTOR_HEAP_TYPE_RTV" },
		{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, L"D3D12_DESCRIPTOR_HEAP_TYPE_DSV" },
		{ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES, L"D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES" }
		};

		ComPtr<ID3D12Device> m_device = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE m_type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		ComPtr<ID3D12DescriptorHeap> m_heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_heapStartCPU = {};
		D3D12_GPU_DESCRIPTOR_HANDLE m_heapStartGPU = {};
		bool m_shaderVisible = false;
		UINT m_descriptorSize = 0;
		UINT m_descriptorCount = 0;
		std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, std::optional<D3D12_GPU_DESCRIPTOR_HANDLE>>> m_handles;

		void CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

		void ResizeHeap(UINT newSize);

		

	public:
		DescriptorHeap(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible = false);
		~DescriptorHeap();

		operator ComPtr<ID3D12DescriptorHeap> () { return m_heap; }
		ComPtr<ID3D12DescriptorHeap> GetHeap() { return m_heap; }

		dxhandle_t Allocate();

		UINT GetDescriptorSize() const { return m_descriptorSize; }
		UINT GetDescriptorCount() const { return m_descriptorCount; }
		size_t GetAllocatedCount() const { return m_handles.size(); }

		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_type; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(dxhandle_t handle) const
		{
			assert(handle < m_descriptorCount);
			return { m_handles.at(handle).first };
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const
		{
			return { m_heap->GetCPUDescriptorHandleForHeapStart() };
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(dxhandle_t handle) const
		{
			assert(handle < m_descriptorCount);
			assert(m_heapStartGPU.ptr != 0); // Ensure this is a shader-visible heap
			return { m_handles.at(handle).second ? *m_handles.at(handle).second : D3D12_GPU_DESCRIPTOR_HANDLE(0) };
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const
		{
			return { m_heap->GetGPUDescriptorHandleForHeapStart() };
		}
	};

	
}

#endif // !__DX12_DESCRIPTORHEAP_HPP__