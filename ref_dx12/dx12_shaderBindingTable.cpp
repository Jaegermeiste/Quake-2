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

bool dx12::ShaderBindingTable::CreateTable(std::wstring shaderName, ComPtr<ID3D12StateObject> pipelineState, byte* rootArguments = nullptr, size_t rootArgumentsSize = 0)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		if (pipelineState)
		{
			// 1. Get the shader binding table properties
			ComPtr<ID3D12StateObjectProperties> pipelineStateProps;
			pipelineState->QueryInterface(IID_PPV_ARGS(&pipelineStateProps));

			// 2. Get shader identifiers
			auto shaderID = pipelineStateProps->GetShaderIdentifier(shaderName.c_str());

			UINT numShaderRecords = 1;
			m_shaderRecordSize = GetShaderIdentifierSize();
			m_bufferSize = numShaderRecords * m_shaderRecordSize;
			
			m_shaderRecords.push_back(ShaderBindingRecord(shaderID, m_shaderRecordSize, rootArguments, rootArgumentsSize));

			m_shaderRecordsBuffer = std::make_unique<byte*>(new byte[m_bufferSize]);
			size_t offset = 0;
			for (auto const& record : m_shaderRecords)
			{
				record.CopyTo(*m_shaderRecordsBuffer + offset);
				offset += m_shaderRecordSize;
			}
			CreateBuffer(*m_shaderRecordsBuffer, m_bufferSize);

			SAFE_RELEASE(m_shaderRecordsBuffer);

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
