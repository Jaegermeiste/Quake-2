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

XMMATRIX GetViewMatrixFromAngles(const XMVECTOR eyePosition, const XMVECTOR viewAngles)
{
	try
	{
		XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);

		// Convert angles (Yaw, Pitch, Roll) to a rotation matrix
		XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(viewAngles.m128_f32[PITCH]),  // Pitch
			XMConvertToRadians(viewAngles.m128_f32[YAW]),    // Yaw
			XMConvertToRadians(viewAngles.m128_f32[ROLL])    // Roll
		);

		forward = XMVector3Transform(forward, rotation);
		up = XMVector3Transform(up, rotation);

		return XMMatrixLookToLH(eyePosition, forward, up);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return XMMatrixIdentity();
}

XMMATRIX dx12::DxrGlobalConstantsBuffer::UpdateViewProjection(XMVECTOR cameraOrigin, XMVECTOR eulerAngles, float fovY, size_t aspectWidth, size_t aspectHeight)
{
	LOG_FUNC();

	try
	{
		// World
		XMStoreFloat4x4(&m_constants.world, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&m_constants.invWorld, XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixIdentity())));

		// Camera
		auto viewMatrix = GetViewMatrixFromAngles(eulerAngles, cameraOrigin);

		if (fovY < 0)
		{
			fovY = 90;
		}
		auto projectionMatrix = XMMatrixPerspectiveFovLH(fovY, static_cast<float>(aspectWidth) / aspectHeight, ref->cvars->zNear3D->Float(), ref->cvars->zFar3D->Float());

		auto viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
				
		XMStoreFloat4x4(&m_constants.viewProjection, XMMatrixTranspose(viewProjectionMatrix));
		XMStoreFloat4x4(&m_constants.invViewProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, viewProjectionMatrix)));

		return viewProjectionMatrix;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return XMMatrixIdentity();
}

void dx12::DxrGlobalConstantsBuffer::UpdateTime(float simulationTime)
{
	LOG_FUNC();

	try
	{
		m_constants.simulationTime.y = m_constants.simulationTime.x; // Last
		m_constants.simulationTime.x = simulationTime;               // Current
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::DxrGlobalConstantsBuffer::UpdateLights(std::vector<dx12::Light> lights)
{
	LOG_FUNC();

	try
	{
		auto null3a = XMFLOAT3A({ 0.0f, 0.0f, 0.0f });
		auto null4a = XMFLOAT4A({ 0.0f, 0.0f, 0.0f, 0.0f });

		// Store as DxrLights
		for (size_t i = 0; i < MAX_DXR_LIGHTS; i++)
		{
			if (i < lights.size())
			{
				XMStoreFloat3A(&m_constants.lights[i].position, lights[i].origin);
				XMStoreFloat4A(&m_constants.lights[i].color, lights[i].color);
				m_constants.lights[i].attenuation.x = lights[i].constantAttenuation;
				m_constants.lights[i].attenuation.y = lights[i].linearAttenuation;
				m_constants.lights[i].attenuation.z = lights[i].quadraticAttenuation;
				m_constants.lights[i].on.x = 1; // True
			}
			else
			{
				m_constants.lights[i].position = null3a;
				m_constants.lights[i].color = null4a;
				m_constants.lights[i].attenuation = null3a;
				m_constants.lights[i].on.x = 0; // False
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

void dx12::DxrGlobalConstantsBuffer::UpdateSun(XMVECTOR origin, XMVECTOR color)
{
	LOG_FUNC();

	try
	{
		XMStoreFloat3A(&m_constants.sunlightOrigin, origin);
		XMStoreFloat4A(&m_constants.sunlightColor, color);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::DxrGlobalConstantsBuffer::UpdateRaytracing(UINT maxRecursionDepth)
{
	LOG_FUNC();

	try
	{
		m_constants.maxRecursionDepth.x = maxRecursionDepth;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

bool dx12::DxrGlobalConstantsBuffer::Create()
{
	LOG_FUNC();

	try
	{
		return CreateConstantBuffer(&m_constants, sizeof(m_constants));
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

void dx12::DxrGlobalConstantsBuffer::Upload(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		if (commandList && commandList->IsOpen())
		{
			UpdateConstantBuffer(commandList, &m_constants, sizeof(m_constants));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
