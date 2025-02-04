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

void dx12::Texture2D::CreateSRV()
{
    // Describe the SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = m_resource->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = m_resource->GetDesc().MipLevels;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    // Get the descriptor heap handle and create the SRV
    m_srvHandle = ref->sys->dx->HeapCBVSRVUAV()->Allocate();
    m_cachedD3D12SRVHandle = ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_srvHandle);
    ref->sys->dx->Device()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cachedD3D12SRVHandle);
}

void dx12::Texture2D::RefreshSRV()
{
    if (m_srvHandle == 0) 
    {
        CreateSRV();
    }
    else if (ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_srvHandle).ptr != m_cachedD3D12SRVHandle.ptr)
    {
        // Describe the SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = m_resource->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = m_resource->GetDesc().MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        // Get the descriptor heap handle and create a fresh SRV
        m_cachedD3D12SRVHandle = ref->sys->dx->HeapCBVSRVUAV()->GetCPUDescriptorHandle(m_srvHandle);
        ref->sys->dx->Device()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_cachedD3D12SRVHandle);
    }
}

void dx12::Texture2D::BindSRV(std::shared_ptr<CommandList> commandList)
{
	if (commandList && commandList->IsOpen())
	{
        RefreshSRV();

		// Bind the SRV descriptor table at root parameter index 1
		commandList->List()->SetGraphicsRootDescriptorTable(1, ref->sys->dx->HeapCBVSRVUAV()->GetGPUDescriptorHandle(m_srvHandle));
	}
}
