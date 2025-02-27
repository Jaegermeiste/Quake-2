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

#ifndef __DX12_3D_HPP__
#define __DX12_3D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	namespace GlobalRootSignatureParams {
		enum Value {
			OutputViewSlot = 0,
			AccelerationStructureSlot,
			SceneConstantSlot,
			Count
		};
	}

	namespace MissRootSignatureParams {
		enum Value {
			CubeTextureSlot,
			CubeConstantSlot,
			Count
		};
	}

	typedef struct missRootConstant_s {
		XMMATRIX                                    m_skyboxRotation = {};
	} missRootConstants_t;

	__declspec(align(16)) class Subsystem3D {
		friend class System;
	private:
		std::shared_ptr<RenderTarget>		m_renderTarget = nullptr;

		ComPtr<ID3D12RootSignature>                 m_globalRootSignature = nullptr;
		ComPtr<ID3D12RootSignature>                 m_rayGenRootSignature = nullptr;
		ComPtr<ID3D12RootSignature>                 m_hitRootSignature = nullptr;
		ComPtr<ID3D12RootSignature>                 m_missRootSignature = nullptr;

		size_t                                      m_globalRootArgumentsSize = 0;
		size_t                                      m_rayGenRootArgumentsSize = 0;
		size_t                                      m_hitGroupRootArgumentsSize = 0;
		size_t                                      m_missRootArgumentsSize = 0;

		ComPtr<ID3D12StateObject>                   m_raytracingPipelineState = nullptr;
		std::shared_ptr<RaytracingBuffer>           m_dxrBuffer = nullptr;
		std::shared_ptr<ShaderBindingTable>         m_rayGenSBT = nullptr;
		std::shared_ptr<ShaderBindingTable>         m_hitGroupSBT = nullptr;
		std::shared_ptr<ShaderBindingTable>         m_missSBT = nullptr;

		std::shared_ptr<CommandList>    	        m_commandList = nullptr;

		dxhandle_t                                  m_3dRTVHandle = {};
		dxhandle_t                                  m_3dDSVHandle = {};
		dxhandle_t                                  m_3dSRVHandle = {};
		D3D12_VIEWPORT                              m_viewport = {};
		D3D12_RECT                                  m_scissorRect = {};

		std::shared_ptr<DxrGlobalConstantsBuffer>   m_globalRootConstants = nullptr;
		missRootConstants_t                         m_missRootConstants = {};
		
		Shader						                m_shaderCommon;
		Shader						                m_shaderRaygen;
		Shader						                m_shaderMiss;
		Shader						                m_shaderHit;

		dxhandle_t				                    m_globalConstantBufferHandle = 0;

		XMVECTOR                                    m_blendColor = DirectX::Colors::Transparent;

		double                                      m_simulationTime = 0.0f;

		float                                       m_skyboxRotationRadPerSec = 0;
		XMVECTOR                                    m_skyboxRotationAxis = {};

		Frustum                                     m_viewFrustum = {};

		std::vector<Light>                          m_dynamicLights;
		std::vector<Particle>                       m_particles;
		std::vector<Entity>                         m_entities;

		size_t                                      GetAlignedRootArgumentSize(const D3D12_ROOT_PARAMETER1& rootParameter);
		size_t                                      GetTotalRootArgumentSize(const D3D12_ROOT_PARAMETER1* rootParameters, size_t numParameters);

		bool                                        CreateRootSignatures();

		bool                                        CreateGlobalConstantBuffer();

		bool                                        CompileShaders();

		bool                                        CreateRaytracingPipelineStateObject();

		bool                                        CreateRaytracingOutputBuffer();

		bool                                        CreateShaderBindingTable();

		bool                                        CreateRenderTarget();

		void                                        UpdateSkyboxRotation();

	public:
									                Subsystem3D();

		bool						                Initialize();

		void                                        SetSkyboxRotation(float rotationSpeedDegreesPerSecond, vec3_t axis);
		void                                        Update(D3D12_RECT scissor, float cameraOrigin[3], float eulerAngles[3], float fovY, double simulationTime, float blendColor[4]);

		void                                        PushDynamicLight(Light light);
		void                                        PushParticle(Particle particle);
		void                                        PushEntity(Entity entity);

		void						                Clear();

		void						                Render(std::shared_ptr<CommandList> commandListSwap);

		void						                Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_3D_HPP__