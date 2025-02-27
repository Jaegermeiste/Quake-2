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



dx12::Subsystem3D::Subsystem3D()
{
	LOG_FUNC();

	try
	{
		m_commandList = nullptr;
		m_renderTarget = nullptr;
		m_globalRootSignature = nullptr;
		m_rayGenRootSignature = nullptr;
		m_hitRootSignature = nullptr;
		m_missRootSignature = nullptr;

		// Avoid resizing on the fly
		m_dynamicLights.reserve(MAX_DLIGHTS);
		m_particles.reserve(MAX_PARTICLES);
		m_entities.reserve(MAX_ENTITIES);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

bool dx12::Subsystem3D::Initialize()
{
	LOG_FUNC();

	try
	{
		if (!CreateRootSignatures())
		{
			LOG(error) << "Failed to create root signatures.";
			return false;
		}

		if (!CompileShaders())
		{
			LOG(error) << "Failed to compile shaders.";
			return false;
		}

		if (!CreateRenderTarget())
		{
			LOG(error) << "Failed to create render target.";
			return false;
		}

		if (!CreateRaytracingPipelineStateObject())
		{
			LOG(error) << "Failed to create raytracing pipeline state object.";
			return false;
		}

		if (!CreateRaytracingOutputBuffer())
		{
			LOG(error) << "Failed to create raytracing output buffer.";
			return false;
		}

		if (!CreateShaderBindingTable())
		{
			LOG(error) << "Failed to create shader binding table.";
			return false;
		}

		m_commandList = std::make_shared<CommandList>(L"3dCommandList", m_globalRootSignature, m_raytracingPipelineState);

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			// Position
			{
				"POSITION",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32A32_FLOAT, // Format
				0,           // InputSlot
				0,           // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Normal
			{
				"NORMAL",     // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Tangent
			{
				"Tangent",     // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32B32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Texture Coordinate 0
			{
				"TEXCOORD0",  // SemanticName
				0,           // SemanticIndex
				DXGI_FORMAT_R32G32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			},
			// Texture Coordinate 1
			{
				"TEXCOORD1",  // SemanticName
				1,           // SemanticIndex
				DXGI_FORMAT_R32G32_FLOAT, // Format
				0,           // InputSlot
				D3D12_APPEND_ALIGNED_ELEMENT, // AlignedByteOffset
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass
				0            // InstanceDataStepRate
			}
		};

		if (m_commandList)
		{
			if (m_renderTarget)
			{
				m_renderTarget->Transition(m_commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}

			m_commandList->Prepare();
		}

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

static inline void NormalizePlane(XMVECTOR& plane) {
	LOG_FUNC();

	try
	{
		XMVECTOR length = XMVector3Length(plane);
		plane /= length;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

static dx12::Frustum ExtractFrustumPlanes(const XMMATRIX& viewProjMatrix) {
	LOG_FUNC();

	dx12::Frustum frustum = {};

	try
	{
		// Left plane
		frustum.planes[0].normal = viewProjMatrix.r[3] + viewProjMatrix.r[0];
		frustum.planes[0].d = XMVectorGetW(frustum.planes[0].normal);
		frustum.planes[0].normal = XMVectorSetW(frustum.planes[0].normal, 0.0f);
		NormalizePlane(frustum.planes[0].normal);

		// Right plane
		frustum.planes[1].normal = viewProjMatrix.r[3] - viewProjMatrix.r[0];
		frustum.planes[1].d = XMVectorGetW(frustum.planes[1].normal);
		frustum.planes[1].normal = XMVectorSetW(frustum.planes[1].normal, 0.0f);
		NormalizePlane(frustum.planes[1].normal);

		// Top plane
		frustum.planes[2].normal = viewProjMatrix.r[3] - viewProjMatrix.r[1];
		frustum.planes[2].d = XMVectorGetW(frustum.planes[2].normal);
		frustum.planes[2].normal = XMVectorSetW(frustum.planes[2].normal, 0.0f);
		NormalizePlane(frustum.planes[2].normal);

		// Bottom plane
		frustum.planes[3].normal = viewProjMatrix.r[3] + viewProjMatrix.r[1];
		frustum.planes[3].d = XMVectorGetW(frustum.planes[3].normal);
		frustum.planes[3].normal = XMVectorSetW(frustum.planes[3].normal, 0.0f);
		NormalizePlane(frustum.planes[3].normal);

		// Near plane
		frustum.planes[4].normal = viewProjMatrix.r[2];
		frustum.planes[4].d = XMVectorGetW(frustum.planes[4].normal);
		frustum.planes[4].normal = XMVectorSetW(frustum.planes[4].normal, 0.0f);
		NormalizePlane(frustum.planes[4].normal);

		// Far plane
		frustum.planes[5].normal = viewProjMatrix.r[3] - viewProjMatrix.r[2];
		frustum.planes[5].d = XMVectorGetW(frustum.planes[5].normal);
		frustum.planes[5].normal = XMVectorSetW(frustum.planes[5].normal, 0.0f);
		NormalizePlane(frustum.planes[5].normal);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return frustum;
}

// Function to check if the sphere is inside the frustum
static bool LightInFrustum(const dx12::Light& light, const dx12::Frustum& frustum)
{
	LOG_FUNC();

	try 
	{
		for (unsigned int i = 0; i < 6; i++)
		{
			XMVECTOR planeNormal = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&frustum.planes[i]));
			float planeDistance = frustum.planes[i].d;

			XMVECTOR sphereCenter = light.origin;
			float radius = light.radius;

			float distance = XMVectorGetX(XMVector3Dot(planeNormal, sphereCenter)) + planeDistance;

			if (distance < -radius)
			{
				// Outside the frustum
				return false;
			}
		}
		// Inside the frustum
		return true;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return true;
}

void dx12::Subsystem3D::SetSkyboxRotation(float rotationSpeedDegreesPerSecond, vec3_t axis)
{
	try
	{
		vec3_t vecZero = { 0.0f, 0.0f, 0.0f };
		if (VectorCompare(axis, vecZero))
		{
			m_skyboxRotationAxis = { 0.0f, 1.0f, 0.0f, 0.0f }; // Up
		}
		else
		{
			// Convert everything to radians
			float x = XMConvertToRadians(axis[0]);
			float y = XMConvertToRadians(axis[1]);
			float z = XMConvertToRadians(axis[2]);
			m_skyboxRotationRadPerSec = XMConvertToRadians(rotationSpeedDegreesPerSecond);

			// Normalize the axis
			// Calculate individual rotation matrices
			XMMATRIX xMatrix = XMMatrixRotationX(x);
			XMMATRIX yMatrix = XMMatrixRotationY(y);
			XMMATRIX zMatrix = XMMatrixRotationZ(z);

			// Combine the rotation matrices (order: Yaw -> Pitch -> Roll)
			XMMATRIX rotation = yMatrix * xMatrix * zMatrix;

			// Extract the axis of rotation from the combined rotation matrix
			m_skyboxRotationAxis = XMVector3Normalize(XMVectorSet(
				rotation.r[0].m128_f32[0] - rotation.r[1].m128_f32[1] - rotation.r[2].m128_f32[2] + 1.0f,
				rotation.r[1].m128_f32[0] + rotation.r[0].m128_f32[1],
				rotation.r[2].m128_f32[0] + rotation.r[0].m128_f32[2],
				0.0f));
		}

		// Calculate the rotation matrix using the axis-angle formula
		m_missRootConstants.m_skyboxRotation = XMMatrixRotationAxis(m_skyboxRotationAxis, 0.0f);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem3D::UpdateSkyboxRotation()
{
	try
	{
		if (m_skyboxRotationRadPerSec != 0.0f)
		{
			// Calculate the rotation matrix using the axis-angle formula
			m_missRootConstants.m_skyboxRotation = XMMatrixRotationAxis(m_skyboxRotationAxis, m_skyboxRotationRadPerSec * m_simulationTime);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem3D::Update(D3D12_RECT scissor, float cameraOrigin[3], float eulerAngles[3], float fovY, double simulationTime, float blendColor[4])
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try {
		m_scissorRect = scissor;

		m_viewport.TopLeftX = m_scissorRect.left;
		m_viewport.TopLeftY = m_scissorRect.top;
		m_viewport.Width = m_scissorRect.right - m_scissorRect.left;
		m_viewport.Height = m_scissorRect.bottom - m_scissorRect.top;

		m_blendColor = XMVectorSet(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);

		m_simulationTime = simulationTime;

		if (m_globalRootConstants)
		{
			m_globalRootConstants->UpdateTime(simulationTime);
		}

		// Lights
		XMVECTOR cameraOriginXMV = XMVectorSet(cameraOrigin[0], cameraOrigin[0], cameraOrigin[0], 1.0f);

		if (m_globalRootConstants)
		{
			auto viewAnglesXMV = XMVectorSet(eulerAngles[PITCH], eulerAngles[YAW], eulerAngles[ROLL], 0.0f);
			auto viewProjectionMatrix = m_globalRootConstants->UpdateViewProjection(cameraOriginXMV, viewAnglesXMV, fovY, m_viewport.Width, m_viewport.Height);
			m_viewFrustum = ExtractFrustumPlanes(viewProjectionMatrix);
		}

		// Calculate distance from each light to the camera and filter out those with early attenuation
		std::vector<std::pair<float, Light>> nearLights;
		Concurrency::parallel_for_each(m_dynamicLights.begin(), m_dynamicLights.end(), [&](const Light& light)
			{
				float distance = XMVectorGetX(XMVector3Length(light.origin - cameraOriginXMV));

				// Only include lights that are in the view frustum
				if (LightInFrustum(light, m_viewFrustum))
				{
					nearLights.emplace_back(distance, light);
				}
			});

		// Sort lights by distance
		Concurrency::parallel_sort(nearLights.begin(), nearLights.end(), [](const auto& lhs, const auto& rhs) {
			return lhs.first < rhs.first;
			});

		// Get the sorted light vector
		std::vector<Light> sortedLights;
		sortedLights.reserve(nearLights.size()); // Reserve space to avoid multiple allocations
		for (const auto& [x, y] : nearLights) {
			sortedLights.push_back(y);
		}

		if (m_globalRootConstants)
		{
			m_globalRootConstants->UpdateLights(sortedLights);
		}

		// Sunlight
		XMVECTOR sunlightOrigin = XMVectorSet( 10000.0f, 10000.0f, 10000.0f, 1.0f );
		XMVECTOR sunlightColor = XMVectorSet( 1.0f, 1.0f, 0.878f, 1.0f ); // Bright yellowish-white

		if (m_globalRootConstants)
		{
			m_globalRootConstants->UpdateSun(sunlightOrigin, sunlightColor);
		}

		// Raytracing Config
		if (ref->cvars->dxrMaxRecursionDepth->Modified())
		{
			if (m_globalRootConstants)
			{
				m_globalRootConstants->UpdateRaytracing(ref->cvars->dxrMaxRecursionDepth->Int());
				ref->cvars->dxrMaxRecursionDepth->SetModified(false);
			}
		}

		if (m_commandList && m_commandList->IsOpen())
		{
			if (m_globalRootConstants)
			{
				// Update the constants buffer.
				m_globalRootConstants->Upload(m_commandList);
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

void dx12::Subsystem3D::PushDynamicLight(Light light)
{
	try
	{
		m_dynamicLights.push_back(light);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem3D::PushParticle(Particle particle)
{
	try
	{
		m_particles.push_back(particle);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem3D::PushEntity(Entity entity)
{
	try
	{
		m_entities.push_back(entity);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

// Function to compute the root argument size from a CD3DX12_ROOT_PARAMETER1
size_t dx12::Subsystem3D::GetAlignedRootArgumentSize(const D3D12_ROOT_PARAMETER1& rootParameter)
{
	size_t argumentSize = 0;

	try
	{
		switch (rootParameter.ParameterType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			// 32-bit constants (4 bytes each)
			argumentSize = rootParameter.Constants.Num32BitValues * sizeof(uint32_t);
			break;

		case D3D12_ROOT_PARAMETER_TYPE_CBV:
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			// CBV, SRV, UAV root descriptors are stored as GPU virtual addresses (8 bytes)
			argumentSize = sizeof(D3D12_GPU_VIRTUAL_ADDRESS);
			break;

		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		{
			// Descriptor table stores a GPU descriptor handle (8 bytes)
			// Compute size by summing all descriptor ranges in the table
			const D3D12_DESCRIPTOR_RANGE1* ranges = rootParameter.DescriptorTable.pDescriptorRanges;
			UINT numRanges = rootParameter.DescriptorTable.NumDescriptorRanges;

			// Each descriptor table range contributes one descriptor handle
			argumentSize = numRanges * sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
		}
		break;

		default:
			argumentSize = 0;
			break;
		}

		// Align each individual root argument size to 32 bytes (DXR shader record alignment requirement)
		argumentSize = AlignUp(argumentSize, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return argumentSize;
}

// Function to compute the total root argument size from an array of D3D12_ROOT_PARAMETER1
size_t dx12::Subsystem3D::GetTotalRootArgumentSize(const D3D12_ROOT_PARAMETER1* rootParameters, size_t numParameters)
{
	size_t totalSize = 0;

	try
	{
		for (size_t i = 0; i < numParameters; ++i)
		{
			totalSize += GetAlignedRootArgumentSize(rootParameters[i]);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return totalSize;
}

bool dx12::Subsystem3D::CreateRootSignatures()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try {
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version supported. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(ref->sys->dx->Device()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Global
		CD3DX12_DESCRIPTOR_RANGE1 globalRanges[1] = {};
		globalRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		CD3DX12_ROOT_PARAMETER1 globalRootParameters[2] = {};
		globalRootParameters[0].InitAsConstantBufferView(0);
		globalRootParameters[1].InitAsDescriptorTable(1, &globalRanges[0], D3D12_SHADER_VISIBILITY_ALL);

		m_globalRootArgumentsSize = GetTotalRootArgumentSize(globalRootParameters, _countof(globalRootParameters));

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC globalRootSignatureDesc = {};

		globalRootSignatureDesc.Init_1_1(_countof(globalRootParameters), globalRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		LOG(info) << "Serializing 3D global root signature...";

		ComPtr<ID3DBlob> globalSignatureBlob = nullptr;
		ComPtr<ID3DBlob> globalErrorBlob = nullptr;
		hr = D3DX12SerializeVersionedRootSignature(&globalRootSignatureDesc, featureData.HighestVersion, &globalSignatureBlob, &globalErrorBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Failed serializing 3D global root signature: " << GetD3D12ErrorMessage(hr);

			if (globalErrorBlob)
			{
				LOG(error) << "3D global root signature serialization error details: " << static_cast<const char*>(globalErrorBlob->GetBufferPointer());
			}

			return false;
		}

		LOG(info) << "Creating 3D global root signature...";

		hr = ref->sys->dx->Device()->CreateRootSignature(0, globalSignatureBlob->GetBufferPointer(), globalSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_globalRootSignature));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 3D global root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_globalRootSignature)
		{
			m_globalRootSignature->SetName(L"3D Global Root Signature");
		}

		// Raygen
		CD3DX12_ROOT_PARAMETER1 rayGenRootParameters[1] = {};
		rayGenRootParameters[0].InitAsConstantBufferView(1);

		m_rayGenRootArgumentsSize = GetTotalRootArgumentSize(rayGenRootParameters, _countof(rayGenRootParameters));

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC raygenRootSignatureDesc = {};

		raygenRootSignatureDesc.Init_1_1(_countof(rayGenRootParameters), rayGenRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

		LOG(info) << "Serializing 3D raygen root signature...";

		ComPtr<ID3DBlob> raygenSignatureBlob = nullptr;
		ComPtr<ID3DBlob> raygenErrorBlob = nullptr;
		hr = D3DX12SerializeVersionedRootSignature(&raygenRootSignatureDesc, featureData.HighestVersion, &raygenSignatureBlob, &raygenErrorBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Failed serializing 3D raygen root signature: " << GetD3D12ErrorMessage(hr);

			if (raygenErrorBlob)
			{
				LOG(error) << "3D raygen root signature serialization error details: " << static_cast<const char*>(raygenErrorBlob->GetBufferPointer());
			}

			return false;
		}

		LOG(info) << "Creating 3D raygen root signature...";

		hr = ref->sys->dx->Device()->CreateRootSignature(0, raygenSignatureBlob->GetBufferPointer(), raygenSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rayGenRootSignature));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 3D raygen root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_rayGenRootSignature)
		{
			m_rayGenRootSignature->SetName(L"3D RayGen Root Signature");
		}

		// Hit
		CD3DX12_DESCRIPTOR_RANGE1 hitRanges[1] = {};
		hitRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		CD3DX12_ROOT_PARAMETER1 hitRootParameters[2] = {};
		hitRootParameters[0].InitAsConstantBufferView(1);
		hitRootParameters[1].InitAsDescriptorTable(1, &hitRanges[0], D3D12_SHADER_VISIBILITY_ALL);

		m_hitGroupRootArgumentsSize = GetTotalRootArgumentSize(hitRootParameters, _countof(hitRootParameters));

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC hitRootSignatureDesc = {};

		hitRootSignatureDesc.Init_1_1(_countof(hitRootParameters), hitRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

		LOG(info) << "Serializing 3D hit root signature...";

		ComPtr<ID3DBlob> hitSignatureBlob = nullptr;
		ComPtr<ID3DBlob> hitErrorBlob = nullptr;
		hr = D3DX12SerializeVersionedRootSignature(&hitRootSignatureDesc, featureData.HighestVersion, &hitSignatureBlob, &hitErrorBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Failed serializing 3D hit root signature: " << GetD3D12ErrorMessage(hr);

			if (hitErrorBlob)
			{
				LOG(error) << "3D hit root signature serialization error details: " << static_cast<const char*>(hitErrorBlob->GetBufferPointer());
			}

			return false;
		}

		LOG(info) << "Creating 3D hit root signature...";

		hr = ref->sys->dx->Device()->CreateRootSignature(0, hitSignatureBlob->GetBufferPointer(), hitSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_hitRootSignature));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 3D hit root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_hitRootSignature)
		{
			m_hitRootSignature->SetName(L"3D Hit Root Signature");
		}

		// Miss
		CD3DX12_DESCRIPTOR_RANGE1 missRanges[1] = {};
		missRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		CD3DX12_ROOT_PARAMETER1 missRootParameters[2] = {};
		missRootParameters[0].InitAsConstantBufferView(1);
		missRootParameters[1].InitAsDescriptorTable(1, &missRanges[0], D3D12_SHADER_VISIBILITY_ALL);

		m_missRootArgumentsSize = GetTotalRootArgumentSize(missRootParameters, _countof(missRootParameters));

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC missRootSignatureDesc = {};

		missRootSignatureDesc.Init_1_1(_countof(missRootParameters), missRootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

		LOG(info) << "Serializing 3D miss root signature...";

		ComPtr<ID3DBlob> missSignatureBlob = nullptr;
		ComPtr<ID3DBlob> missErrorBlob = nullptr;
		hr = D3DX12SerializeVersionedRootSignature(&missRootSignatureDesc, featureData.HighestVersion, &missSignatureBlob, &missErrorBlob);

		if (FAILED(hr))
		{
			LOG(error) << "Failed serializing 3D miss root signature: " << GetD3D12ErrorMessage(hr);

			if (missErrorBlob)
			{
				LOG(error) << "3D miss root signature serialization error details: " << static_cast<const char*>(missErrorBlob->GetBufferPointer());
			}

			return false;
		}

		LOG(info) << "Creating 3D miss root signature...";

		hr = ref->sys->dx->Device()->CreateRootSignature(0, missSignatureBlob->GetBufferPointer(), missSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_missRootSignature));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 3D miss root signature: " << GetD3D12ErrorMessage(hr);
			return false;
		}

		if (m_missRootSignature)
		{
			m_missRootSignature->SetName(L"3D Miss Root Signature");
		}

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

bool dx12::Subsystem3D::CreateGlobalConstantBuffer()
{
	LOG_FUNC();

	try
	{
		// Create the constants buffer
		m_globalRootConstants = ref->res->CreateResource<DxrGlobalConstantsBuffer>(L"dxrGlobalConstantsBuffer");

		if (m_globalRootConstants)
		{
			m_globalConstantBufferHandle = m_globalRootConstants->GetHandle();

			auto nullXMV = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

			auto vpm = m_globalRootConstants->UpdateViewProjection(nullXMV, nullXMV, 90, m_renderTarget->Width(), m_renderTarget->Height());

			m_viewFrustum = ExtractFrustumPlanes(vpm);

			m_globalRootConstants->UpdateTime(0.0f);

			m_globalRootConstants->UpdateLights(std::vector<Light>());

			XMVECTOR sunlightOrigin = XMVectorSet(10000.0f, 10000.0f, 10000.0f, 1.0f);
			XMVECTOR sunlightColor = XMVectorSet(1.0f, 1.0f, 0.878f, 1.0f); // Bright yellowish-white

			m_globalRootConstants->UpdateSun(sunlightOrigin, sunlightColor);

			m_globalRootConstants->UpdateRaytracing(ref->cvars->dxrMaxRecursionDepth->Int());

			return m_globalRootConstants->Create();
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

bool dx12::Subsystem3D::CompileShaders()
{
	LOG_FUNC();

	try
	{
		LOG(info) << "Compiling DXR Common Shader...";

		if (!m_shaderCommon.Compile(ref->cvars->shaderCommon3D->String(), SHADER_TARGET_DXR_COMMON))
		{
			LOG(error) << "Unable to compile DXR Common Shader!";
			return false;
		}

		LOG(info) << "Compiling DXR Ray Generation Shader...";

		if (!m_shaderRaygen.Compile(ref->cvars->shaderRaygen3D->String(), SHADER_TARGET_DXR_RAYGEN))
		{
			LOG(error) << "Unable to compile DXR Ray Generation Shader!";
			return false;
		}

		LOG(info) << "Compiling DXR Miss Shader...";

		if (!m_shaderMiss.Compile(ref->cvars->shaderMiss3D->String(), SHADER_TARGET_DXR_MISS))
		{
			LOG(error) << "Unable to compile DXR Miss Shader!";
			return false;
		}

		LOG(info) << "Compiling DXR Hit Shader...";

		if (!m_shaderHit.Compile(ref->cvars->shaderHit3D->String(), SHADER_TARGET_DXR_HIT))
		{
			LOG(error) << "Unable to compile DXR Hit Shader!";
			return false;
		}

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

bool dx12::Subsystem3D::CreateRaytracingPipelineStateObject()
{
	LOG_FUNC();

	try
	{
		HRESULT hr = E_UNEXPECTED;

		// Create shader libraries
		D3D12_DXIL_LIBRARY_DESC dxilLibDescs[3] = {};
		D3D12_STATE_SUBOBJECT subobjects[15] = {};

		dxilLibDescs[0].DXILLibrary = { m_shaderRaygen.DxcBlob()->GetBufferPointer(), m_shaderRaygen.DxcBlob()->GetBufferSize() };
		dxilLibDescs[0].NumExports = 1;
		D3D12_EXPORT_DESC rayGenExport = {};
		rayGenExport.Name = L"RayGen";
		dxilLibDescs[0].pExports = &rayGenExport;
		subobjects[0] = {};
		subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobjects[0].pDesc = &dxilLibDescs[0];

		dxilLibDescs[1].DXILLibrary = { m_shaderHit.DxcBlob()->GetBufferPointer(), m_shaderHit.DxcBlob()->GetBufferSize() };
		dxilLibDescs[1].NumExports = 1;
		D3D12_EXPORT_DESC closestHitExport = {};
		closestHitExport.Name = L"ClosestHit";
		dxilLibDescs[1].pExports = &closestHitExport;
		subobjects[1] = {};
		subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobjects[1].pDesc = &dxilLibDescs[1];

		dxilLibDescs[2].DXILLibrary = { m_shaderMiss.DxcBlob()->GetBufferPointer(), m_shaderMiss.DxcBlob()->GetBufferSize() };
		dxilLibDescs[2].NumExports = 1;
		D3D12_EXPORT_DESC missExport = {};
		missExport.Name = L"Miss";
		dxilLibDescs[2].pExports = &missExport;
		subobjects[2] = {};
		subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobjects[2].pDesc = &dxilLibDescs[2];

		// Define the hit group
		D3D12_HIT_GROUP_DESC hitGroupDesc = {};
		hitGroupDesc.HitGroupExport = L"HitGroup";
		hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroupDesc.ClosestHitShaderImport = L"ClosestHit";

		subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		subobjects[3].pDesc = &hitGroupDesc;

		// Root signatures
		auto globalRootSigPtr = m_globalRootSignature.Get();

		subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		subobjects[4].pDesc = &globalRootSigPtr;

		auto rayGenRootSigPtr = m_rayGenRootSignature.Get();

		subobjects[5].Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		subobjects[5].pDesc = &rayGenRootSigPtr;

		D3D12_RAYTRACING_SHADER_CONFIG rayGenShaderConfig = {};
		rayGenShaderConfig.MaxPayloadSizeInBytes = sizeof(Vertex3D);
		rayGenShaderConfig.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

		subobjects[6].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobjects[6].pDesc = &rayGenShaderConfig;

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION rayGenRootSigAssoc = {};
		rayGenRootSigAssoc.NumExports = 1;
		rayGenRootSigAssoc.pExports = new LPCWSTR[1]{ L"RayGen" };
		rayGenRootSigAssoc.pSubobjectToAssociate = &subobjects[5];

		subobjects[7].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobjects[7].pDesc = &rayGenRootSigAssoc;

		auto hitGroupRootSigPtr = m_hitRootSignature.Get();

		subobjects[8].Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		subobjects[8].pDesc = &hitGroupRootSigPtr;

		D3D12_RAYTRACING_SHADER_CONFIG hitGroupShaderConfig = {};
		hitGroupShaderConfig.MaxPayloadSizeInBytes = sizeof(Vertex3D);
		hitGroupShaderConfig.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

		subobjects[9].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobjects[9].pDesc = &hitGroupShaderConfig;

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION hitGroupRootSigAssoc = {};
		hitGroupRootSigAssoc.NumExports = 1;
		hitGroupRootSigAssoc.pExports = new LPCWSTR[1]{ L"ClosestHit" };
		hitGroupRootSigAssoc.pSubobjectToAssociate = &subobjects[8];

		subobjects[10].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobjects[10].pDesc = &hitGroupRootSigAssoc;

		auto missRootSigPtr = m_missRootSignature.Get();

		subobjects[11].Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		subobjects[11].pDesc = &missRootSigPtr;

		D3D12_RAYTRACING_SHADER_CONFIG missShaderConfig = {};
		missShaderConfig.MaxPayloadSizeInBytes = sizeof(Vertex3D);
		missShaderConfig.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

		subobjects[12].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobjects[12].pDesc = &missShaderConfig;

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION missRootSigAssoc = {};
		missRootSigAssoc.NumExports = 1;
		missRootSigAssoc.pExports = new LPCWSTR[1]{ L"Miss" };
		missRootSigAssoc.pSubobjectToAssociate = &subobjects[11];

		subobjects[13].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobjects[13].pDesc = &missRootSigAssoc;

		// Pipeline configuration
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 1;

		subobjects[14].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		subobjects[14].pDesc = &pipelineConfig;

		// Assemble the state object
		D3D12_STATE_OBJECT_DESC raytracingPipelineDesc = {};
		raytracingPipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		raytracingPipelineDesc.NumSubobjects = _countof(subobjects);
		raytracingPipelineDesc.pSubobjects = subobjects;

		// Create the state object
		hr = ref->sys->dx->Device()->CreateStateObject(&raytracingPipelineDesc, IID_PPV_ARGS(&m_raytracingPipelineState));

		if (FAILED(hr))
		{
			LOG(error) << "Unable to create 3D raytracing pipeline state object: " << GetD3D12ErrorMessage(hr);
			return false;
		}

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

bool dx12::Subsystem3D::CreateRaytracingOutputBuffer()
{
	LOG_FUNC();

	try
	{
		if (m_dxrBuffer)
		{
			SAFE_RELEASE(m_dxrBuffer);
		}

		m_dxrBuffer = ref->res->GetOrCreateResource<RaytracingBuffer>(L"dxrRaytracingBuffer");

		if (!m_dxrBuffer)
		{
			LOG(error) << "Failed to obtain DXR buffer resource!";

			return false;
		}

		if (!m_renderTarget || !m_dxrBuffer->CreateBuffer(m_renderTarget->Width(), m_renderTarget->Height()))
		{
			LOG(error) << "Failed to create DXR buffer!";

			return false;
		}

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

bool dx12::Subsystem3D::CreateShaderBindingTable()
{
	LOG_FUNC();

	try
	{
		m_rayGenSBT = ref->res->GetOrCreateResource<ShaderBindingTable>(L"dxrRayGenShaderBindingTable");
		m_hitGroupSBT = ref->res->GetOrCreateResource<ShaderBindingTable>(L"dxrHitGroupShaderBindingTable");
		m_missSBT = ref->res->GetOrCreateResource<ShaderBindingTable>(L"dxrMissShaderBindingTable");

		if ((!m_rayGenSBT) || (!m_hitGroupSBT) || (!m_missSBT))
		{
			LOG(error) << "Failed to obtain DXR shader binding table resource!";

			return false;
		}

		/*if ((!m_raytracingPipelineState) ||
			(!m_rayGenSBT->CreateTable(m_raytracingPipelineState, m_globalRootArgumentsSize, m_rayGenRootArgumentsSize, m_hitGroupRootArgumentsSize, m_missRootArgumentsSize)))
		{
			LOG(error) << "Failed to create DXR shader binding table!";

			return false;
		}*/
		
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

bool dx12::Subsystem3D::CreateRenderTarget()
{
	LOG_FUNC();

	try
	{
		if (m_renderTarget)
		{
			SAFE_RELEASE(m_renderTarget);
		}

		m_renderTarget = ref->res->GetOrCreateResource<RenderTarget>(L"3dRenderTarget");

		if (!m_renderTarget)
		{
			LOG(error) << "Failed to obtain 3d Render Target resource!";

			return false;
		}

		if (!m_renderTarget->CreateRenderTarget(ref->sys->dx->m_modeWidth, ref->sys->dx->m_modeHeight, DirectX::Colors::Black))
		{
			LOG(error) << "Failed to create 3d Render Target!";

			return false;
		}

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

void dx12::Subsystem3D::Clear()
{
	LOG_FUNC();

	try
	{
		m_dynamicLights.clear();
		m_particles.clear();
		m_entities.clear();

		if (m_commandList && m_commandList->IsOpen() && m_renderTarget)
		{
			LOG(trace) << "Clearing 3D RenderTargetView.";

			m_renderTarget->Clear(m_commandList);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Subsystem3D::Render(std::shared_ptr<CommandList> commandListSwap)
{
	LOG_FUNC();

	try
	{
		/*if (m_commandList && m_commandList->IsOpen() && m_dxrSBT && m_renderTarget)
		{
			// Define dispatch description
			D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = {};
			dispatchRaysDesc.Width = static_cast<UINT>(m_renderTarget->Width());
			dispatchRaysDesc.Height = static_cast<UINT>(m_renderTarget->Height());
			dispatchRaysDesc.Depth = 1;

			dispatchRaysDesc.RayGenerationShaderRecord.StartAddress = m_dxrSBT->GetRayGenShaderGPUVirtualAddress();
			dispatchRaysDesc.RayGenerationShaderRecord.SizeInBytes = m_dxrSBT->GetRayGenShaderSize();

			dispatchRaysDesc.MissShaderTable.StartAddress = m_dxrSBT->GetMissShaderGPUVirtualAddress();
			dispatchRaysDesc.MissShaderTable.SizeInBytes = m_dxrSBT->GetMissShaderSize();
			dispatchRaysDesc.MissShaderTable.StrideInBytes = m_dxrSBT->GetMissShaderStride();

			dispatchRaysDesc.HitGroupTable.StartAddress = m_dxrSBT->GetHitShaderGPUVirtualAddress();
			dispatchRaysDesc.HitGroupTable.SizeInBytes = m_dxrSBT->GetHitShaderSize();
			dispatchRaysDesc.HitGroupTable.StrideInBytes = m_dxrSBT->GetHitShaderStride();

			m_commandList->SetDXRPipelineState(m_raytracingPipelineState.Get());
			m_commandList->List()->DispatchRays(&dispatchRaysDesc);
		}*/

		if (commandListSwap && commandListSwap->IsOpen())
		{
			// Render to the Render Target
			if (m_commandList && m_commandList->IsOpen())
			{
				m_commandList->Close();
				m_commandList->Execute();

				if (m_renderTarget)
				{
					m_renderTarget->Transition(commandListSwap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

					// Bind the SRV
					commandListSwap->List()->SetGraphicsRootDescriptorTable(1, m_renderTarget->GetSRVGPUDescriptorHandle());

					// Render the render target to the backbuffer
					m_renderTarget->RenderQuad(commandListSwap);

					m_renderTarget->Transition(commandListSwap, D3D12_RESOURCE_STATE_RENDER_TARGET);
				}

				m_commandList->Prepare();
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

void dx12::Subsystem3D::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	try
	{
		m_dynamicLights.clear();
		m_dynamicLights.shrink_to_fit();

		m_particles.clear();
		m_particles.shrink_to_fit();

		m_entities.clear();
		m_entities.shrink_to_fit();

		if (m_commandList)
		{
			m_commandList->Close();
		}

		ref->sys->dx->WaitForGPU();

		SAFE_RELEASE(m_commandList);

		SAFE_RELEASE(m_raytracingPipelineState);

		SAFE_RELEASE(m_globalRootSignature);

		SAFE_RELEASE(m_rayGenRootSignature);

		SAFE_RELEASE(m_hitRootSignature);

		SAFE_RELEASE(m_missRootSignature);

		m_shaderCommon.Shutdown();

		m_shaderRaygen.Shutdown();

		m_shaderMiss.Shutdown();

		m_shaderHit.Shutdown();

		//SAFE_RELEASE(m_dxrSBT);

		SAFE_RELEASE(m_dxrBuffer);

		SAFE_RELEASE(m_renderTarget);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	LOG(info) << "Shutdown complete.";
}
