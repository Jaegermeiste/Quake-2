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

#ifndef __DX12_DXRGLOBALCONSTANTSBUFFER_HPP__
#define __DX12_DXRGLOBALCONSTANTSBUFFER_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	__declspec(align(16)) struct DxrLight
	{
		XMFLOAT3A position = {};
		XMFLOAT4A color = {};
		XMFLOAT3A attenuation = {}; // Constant, Linear, Quadratic
		XMINT4    on = {};
	};

	constexpr auto MAX_DXR_LIGHTS = 8;

	typedef __declspec(align(16)) struct DxrGlobalConstants_s
	{
		// Transformation Matrices
		XMFLOAT4X4A world;
		XMFLOAT4X4A invWorld;

		// Camera
		XMFLOAT4X4A viewProjection;
		XMFLOAT4X4A invViewProjection;

		// Sunlight
		XMFLOAT3A   sunlightOrigin;
		XMFLOAT4A   sunlightColor;

		// Lights
		DxrLight    lights[MAX_DXR_LIGHTS];
		
		// Ray Tracing Configuration
		XMINT4      maxRecursionDepth = {};
		//UINT payloadSize;

		// Time
		XMFLOAT2A   simulationTime;
	} DxrGlobalConstants_t;

	class DxrGlobalConstantsBuffer : public ResourceBuffer
	{
		friend class ResourceManager;
	private:
		DxrGlobalConstants_t      m_constants;

	public:
		DxrGlobalConstantsBuffer(std::wstring name) : ResourceBuffer(name)
		{
			m_type = RESOURCE_CONSTANTBUFFER;
			m_count = 0;
		};

		bool        CreateConstantBuffer(DxrGlobalConstants_t* bufferData, size_t bufferSize) 
		{
			return CreateBuffer(bufferData, bufferSize);
		}

		bool        UpdateConstantBuffer(std::shared_ptr<CommandList> commandList, DxrGlobalConstants_t* bufferData, size_t bufferSize) 
		{
			return UpdateBuffer(commandList, bufferData, bufferSize);
		}

		XMMATRIX    UpdateViewProjection(XMVECTOR cameraOrigin, XMVECTOR eulerAngles, float fovY, size_t aspectWidth, size_t aspectHeight);
		void        UpdateTime(float simulationTime);
		void        UpdateLights(std::vector<dx12::Light> lights);
		void        UpdateSun(XMVECTOR origin, XMVECTOR color);
		void        UpdateRaytracing(UINT maxRecursionDepth);

		bool        Create();
		void        Upload(std::shared_ptr<CommandList> commandList);

		size_t		ConstantCount() { return m_count; };
	};
}

template bool dx12::ResourceBuffer::CreateBuffer<dx12::DxrGlobalConstants_t>(dx12::DxrGlobalConstants_t* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UploadBuffer<dx12::DxrGlobalConstants_t>(dx12::DxrGlobalConstants_t* bufferData, size_t bufferSize);
template bool dx12::ResourceBuffer::UpdateBuffer<dx12::DxrGlobalConstants_t>(std::shared_ptr<CommandList> commandList, dx12::DxrGlobalConstants_t* bufferData, size_t bufferSize);

template std::shared_ptr<dx12::DxrGlobalConstantsBuffer> dx12::ResourceManager::CreateResource<dx12::DxrGlobalConstantsBuffer>(std::wstring name);
template std::shared_ptr<dx12::DxrGlobalConstantsBuffer> dx12::ResourceManager::GetOrCreateResource<dx12::DxrGlobalConstantsBuffer>(std::wstring name);
template std::shared_ptr<dx12::DxrGlobalConstantsBuffer> dx12::ResourceManager::GetResource<dx12::DxrGlobalConstantsBuffer>(std::wstring name);
template std::shared_ptr<dx12::DxrGlobalConstantsBuffer> dx12::ResourceManager::GetResource<dx12::DxrGlobalConstantsBuffer>(dxhandle_t handle);

#endif//__DX12_DXRGLOBALCONSTANTSBUFFER_HPP__