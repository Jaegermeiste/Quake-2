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

bool dx12::BLASBuffer::Create(std::shared_ptr<CommandList> commandList, dxhandle_t vertexBufferResourceHandle, dxhandle_t indexBufferResourceHandle)
{
	LOG_FUNC();

	try
	{

		if (commandList)
		{
			auto vertexBuffer = ref->res->GetResource<VertexBuffer>(vertexBufferResourceHandle);
			auto indexBuffer = ref->res->GetResource<IndexBuffer>(indexBufferResourceHandle);

			if (vertexBuffer && indexBuffer)
			{
				m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

				// Vertex buffer
				m_geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
				m_geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex3D);
				m_geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer->VertexCount());
				m_geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

				// Index buffer
				m_geometryDesc.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
				m_geometryDesc.Triangles.IndexCount = static_cast<UINT>(indexBuffer->IndexCount());
				m_geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

				// Transform
				m_geometryDesc.Triangles.Transform3x4 = 0; // No transformation

				m_blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
				m_blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
				m_blasInputs.NumDescs = 1;
				m_blasInputs.pGeometryDescs = &m_geometryDesc;
				m_blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

				ref->sys->dx->Device()->GetRaytracingAccelerationStructurePrebuildInfo(&m_blasInputs, &m_blasPrebuildInfo);

				return CreateBuffers(commandList);
			}
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

bool dx12::BLASBuffer::BuildBuffers(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try {
		if (commandList && commandList->IsOpen())
		{
			if (m_resource && m_scratch)
			{
				HRESULT hr = E_UNEXPECTED;

				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {};
				blasDesc.Inputs = m_blasInputs;
				blasDesc.ScratchAccelerationStructureData = m_scratch->GetGPUVirtualAddress();
				blasDesc.DestAccelerationStructureData = m_resource->GetGPUVirtualAddress();

				commandList->List()->BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);

				// Insert a UAV barrier to ensure completion
				D3D12_RESOURCE_BARRIER uavBarrier = {};
				uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				uavBarrier.UAV.pResource = m_resource.Get();
				commandList->List()->ResourceBarrier(1, &uavBarrier);

#ifdef _DEBUG
				DumpD3DDebugMessagesToLog();
#endif

				return true;
			}
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

bool dx12::BLASBuffer::CreateBuffers(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try {
		HRESULT hr = E_UNEXPECTED;

		m_count = 1;

		// Create BLAS buffer
		auto blasHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto blasResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_blasPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		hr = ref->sys->dx->Device()->CreateCommittedResource(
			&blasHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&blasResourceDesc,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
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

		m_scratchCount = 1;

		// Create scratch buffer
		auto scratchHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto scratchResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_blasPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		hr = ref->sys->dx->Device()->CreateCommittedResource(
			&scratchHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&scratchResourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_scratch)
		);

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create scratch resource: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_scratch)
		{
			m_scratch->SetName(m_name.c_str());
		}

		BuildBuffers(commandList);

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


