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

dx12::BottomLevelAccelerationStructure::BottomLevelAccelerationStructure()
{
	LOG_FUNC();

	m_blasBuffer = nullptr;
}

dx12::BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
	LOG_FUNC();
}

bool dx12::BottomLevelAccelerationStructure::CreateGeometryBuffers(Vertex3D* vertices, size_t vertexBufferSize, unsigned long* indices, size_t indexBufferSize)
{
	LOG_FUNC();

	try
	{
		return CreateBuffers(vertices, vertexBufferSize, indices, indexBufferSize);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::BottomLevelAccelerationStructure::UpdateVertexBuffer(std::shared_ptr<CommandList> commandList, Vertex3D* vertices, size_t bufferSize)
{
	LOG_FUNC();

	try
	{
		return __super::UpdateVertexBuffer(commandList, vertices, bufferSize);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::BottomLevelAccelerationStructure::UpdateIndexBuffer(std::shared_ptr<CommandList> commandList, unsigned long* indices, size_t bufferSize)
{
	LOG_FUNC();


	try
	{
		return __super::UpdateIndexBuffer(commandList, indices, bufferSize);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

bool dx12::BottomLevelAccelerationStructure::CreateBLAS(std::shared_ptr<CommandList> commandList)
{
	LOG_FUNC();

	try
	{
		if (m_vertexBuffer && m_indexBuffer)
		{
			m_blasBuffer = ref->res->GetOrCreateResource<BLASBuffer>(L"BLAS_" + GetUUID());

			return m_blasBuffer->Create(commandList, m_vertexBuffer->GetHandle(), m_indexBuffer->GetHandle());
		}

		return false;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

void dx12::BottomLevelAccelerationStructure::Shutdown()
{
	try
	{
		if (m_blasBuffer) {
			SAFE_RELEASE(m_blasBuffer);
		}

		if (m_indexBuffer) {
			SAFE_RELEASE(m_indexBuffer);
		}

		if (m_vertexBuffer) {
			SAFE_RELEASE(m_vertexBuffer);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
