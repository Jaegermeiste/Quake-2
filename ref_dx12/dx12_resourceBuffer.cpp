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

template <typename T>
bool dx12::ResourceBuffer::UploadBuffer(T* bufferData, size_t bufferSize)
{
	LOG_FUNC();

	try {
		if (m_resource)
		{
			HRESULT hr = E_UNEXPECTED;

			CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU

			UINT8* pDataBegin = nullptr;
			//hr = m_resource->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin));
			hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&pDataBegin));

			if (FAILED(hr))
			{
				LOG(error) << "Failed to lock buffer: " << GetD3D12ErrorMessage(hr);
				return false;
			}

			memcpy(pDataBegin, bufferData, bufferSize);

			// Unmap the buffer
			D3D12_RANGE writeRange = { 0, bufferSize };
			//m_resource->Unmap(0, &writeRange);
			m_resource->Unmap(0, nullptr);

#ifdef _DEBUG
			DumpD3DDebugMessagesToLog();
#endif

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

template <typename T>
bool dx12::ResourceBuffer::UpdateBuffer(std::shared_ptr<CommandList> commandList, T* bufferData, size_t bufferSize)
{
	LOG_FUNC();

	bool retVal = false;

	try
	{

		if (commandList && commandList->IsOpen())
		{
			// Assume commandList is a valid ID3D12GraphicsCommandList pointer
			/*D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = m_resource.Get();

			if (m_type == RESOURCE_INDEXBUFFER)
			{
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
			}
			else
			{
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			}

			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;*/

			// Transition to COPY_DEST
			//commandList->List()->ResourceBarrier(1, &barrier);

			// Perform the update...
			retVal = UploadBuffer(bufferData, bufferSize);

			//commandList->Execute();
			//commandList->Prepare();

			// Transition back
			/*barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;


			if (m_type == RESOURCE_INDEXBUFFER)
			{
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
			}
			else
			{
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			}*/

			//commandList->List()->ResourceBarrier(1, &barrier);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return retVal;
}

template <typename T>
bool dx12::ResourceBuffer::CreateBuffer(T* bufferData, size_t bufferSize)
{
	LOG_FUNC();

	try {
		HRESULT hr = E_UNEXPECTED;

		m_count = bufferSize / sizeof(T);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		// Create buffer
		D3D12_RESOURCE_DESC1 bufferDesc = {};
		
		if (ref->sys->dx->HasTightAlignment())
		{
			bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize, D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT);
		}
		else
		{
			bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize);
		}
		hr = ref->sys->dx->Device()->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_resource)
		);

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create buffer resource: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_resource)
		{
			m_resource->SetName(m_name.c_str());
		}

		if (!UploadBuffer(bufferData, bufferSize))
		{
			return false;
		}

#ifdef _DEBUG
		DumpD3DDebugMessagesToLog();
#endif

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


