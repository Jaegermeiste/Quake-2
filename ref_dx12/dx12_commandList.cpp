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
2025Bleeding Eye Studios
*/

#include "dx12_local.hpp"

dx12::CommandList::CommandList(std::wstring name, ComPtr<ID3D12RootSignature> rootSignature, ComPtr<ID3D12PipelineState> pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, dxhandle_t constantBufferViewHandle, dxhandle_t renderTargetViewHandle, dxhandle_t depthStencilViewHandle)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		m_name = name;
		m_rootSignature = rootSignature;
		m_pipelineState = pipelineState;
		m_viewport = viewport;
		m_scissorRect = scissorRect;
		m_cbvHandle = constantBufferViewHandle;
		m_rtvHandle = renderTargetViewHandle;
		m_dsvHandle = depthStencilViewHandle;
		m_dxr = false;
		m_dxrPipelineState = nullptr;

		hr = ref->sys->dx->m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create Command Allocator" << m_name << ": " << GetD3D12ErrorMessage(hr);
		}

		m_commandAllocator->SetName(m_name.c_str());

		LOG(info) << "Creating Command List" << m_name << "...";

		// Create command list
		hr = ref->sys->dx->m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create Command List" << m_name << ": " << GetD3D12ErrorMessage(hr);
		}

		m_commandList->SetName(m_name.c_str());

		// Command lists are created by CreateCommandList in the recording state, so close it now.
		m_commandListOpen = true;

		Close();

		Prepare();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

dx12::CommandList::CommandList(std::wstring name, ComPtr<ID3D12RootSignature> globalRootSignature, ComPtr<ID3D12StateObject> dxrPipelineState)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		m_name = name;
		m_rootSignature = globalRootSignature;
		m_pipelineState = nullptr;
		m_viewport = {};
		m_scissorRect = {};
		m_cbvHandle = 0;
		m_rtvHandle = 0;
		m_dsvHandle = 0;
		m_dxr = true;
		m_dxrPipelineState = dxrPipelineState;

		hr = ref->sys->dx->Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create Command Allocator" << m_name << ": " << GetD3D12ErrorMessage(hr);
		}

		m_commandAllocator->SetName(m_name.c_str());

		LOG(info) << "Creating Command List" << m_name << "...";

		// Create command list
		hr = ref->sys->dx->Device()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create Command List" << m_name << ": " << GetD3D12ErrorMessage(hr);
		}

		m_commandList->SetName(m_name.c_str());

		// Command lists are created by CreateCommandList1 in the closed state.
		m_commandListOpen = false;

		Close();

		Prepare();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::CommandList::Prepare()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		if (m_commandList && !m_commandListOpen)
		{
			ref->sys->dx->WaitForGPU();

			if (m_commandAllocator)
			{
				hr = m_commandAllocator->Reset();

				if (FAILED(hr)) {
					LOG(error) << "Unable to reset Command Allocator: " << GetD3D12ErrorMessage(hr);
				}
			}

			hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);

			if (FAILED(hr)) {
				LOG(error) << "Unable to reset Command List " << m_name << ": " << GetD3D12ErrorMessage(hr);
			}
			else
			{
				m_commandListOpen = true;
			}

			if (!m_dxr)
			{
				// Rasterization
				// 
				// Set necessary state.
				m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

				ID3D12DescriptorHeap* heaps[] = { ref->sys->dx->HeapCBVSRVUAV()->GetHeap().Get() };
				m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

				if (m_cbvHandle != 0)
				{
					m_commandList->SetGraphicsRootConstantBufferView(0, ref->res->GetResource<ConstantBuffer2D>(m_cbvHandle)->GetGPUVirtualAddress());
				}

				m_commandList->RSSetViewports(1, &m_viewport);
				m_commandList->RSSetScissorRects(1, &m_scissorRect);

				if (m_rtvHandle != 0)
				{
					auto rtvHandle = ref->sys->dx->m_descriptorHeapRTV->GetCPUDescriptorHandle(m_rtvHandle);

					if (m_dsvHandle != 0)
					{
						auto dsvHandle = ref->sys->dx->m_descriptorHeapRTV->GetCPUDescriptorHandle(m_dsvHandle);
						m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
					}
					else
					{
						m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
					}
				}

				SetPipelineState(nullptr);
			}
			else
			{
				// DXR
				m_commandList->SetComputeRootSignature(m_rootSignature.Get());

				ID3D12DescriptorHeap* heaps[] = { ref->sys->dx->HeapCBVSRVUAV()->GetHeap().Get() };
				m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

				SetDXRPipelineState(nullptr);
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

void dx12::CommandList::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	LOG_FUNC();

	try
	{
		if (pipelineState)
		{
			m_commandList->SetPipelineState(pipelineState.Get());
		}
		else
		{
			m_commandList->SetPipelineState(m_pipelineState.Get());
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::CommandList::SetDXRPipelineState(ComPtr<ID3D12StateObject> pipelineState)
{
	LOG_FUNC();

	try
	{
		if (pipelineState)
		{
			m_commandList->SetPipelineState1(pipelineState.Get());
		}
		else
		{
			m_commandList->SetPipelineState1(m_dxrPipelineState.Get());
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::CommandList::Execute()
{
	LOG_FUNC();

	try
	{
		if (ref->sys->dx->m_commandQueue && m_commandList)
		{
			Close();

			// Execute all pending commands
			ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
			ref->sys->dx->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			ref->sys->dx->WaitForGPU();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::CommandList::Close()
{
	LOG_FUNC();

	try
	{
		if (m_commandList && m_commandListOpen)
		{
			HRESULT hr = E_UNEXPECTED;

			hr = m_commandList->Close();

			if (FAILED(hr)) {
				LOG(error) << "Unable to close Command List" << m_name << ": " << GetD3D12ErrorMessage(hr);
			}
			else
			{
				m_commandListOpen = false;
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

void dx12::CommandList::Shutdown()
{
	LOG_FUNC();

	try
	{
		Close();

		ref->sys->dx->WaitForGPU();

		SAFE_RELEASE(m_commandList);

		SAFE_RELEASE(m_commandAllocator);

		SAFE_RELEASE(m_pipelineState);

		SAFE_RELEASE(m_dxrPipelineState);

		SAFE_RELEASE(m_rootSignature);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
