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
ref_dx11
2017 Bleeding Eye Studios
*/

#ifndef __DX11_SHADER_HPP__
#define __DX11_SHADER_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	typedef std::string shaderTarget;

#define SHADER_TARGET_VERTEX	"vs"
#define SHADER_TARGET_PIXEL		"ps"
#define SHADER_TARGET_COMPUTE	"cs"
#define SHADER_TARGET_DOMAIN	"ds"
#define SHADER_TARGET_GEOMETRY	"gs"
#define SHADER_TARGET_HULL		"hs"
#define SHADER_TARGET_LIBRARY	"lib"

#define SHADER_ENTRY_POINT_VERTEX	"VS_Entry"
#define SHADER_ENTRY_POINT_PIXEL	"PS_Entry"
#define SHADER_ENTRY_POINT_COMPUTE	"CS_Entry"
#define SHADER_ENTRY_POINT_DOMAIN	"DS_Entry"
#define SHADER_ENTRY_POINT_GEOMETRY	"GS_Entry"
#define SHADER_ENTRY_POINT_HULL		"HS_Entry"
#define SHADER_ENTRY_POINT_LIBRARY	"LIB_Entry"

	class Shader 
	{
	private:
		struct MatrixBufferType
		{
			XMMATRIX world;
			XMMATRIX view;
			XMMATRIX projection;
		};

		ID3D11VertexShader*			m_vertexShader = nullptr;
		ID3D11PixelShader*			m_pixelShader = nullptr;
		ID3D11InputLayout*			m_layout = nullptr;
		ID3D11Buffer*				m_matrixBuffer = nullptr;
		ID3D11SamplerState*			m_sampleState = nullptr;

		void						OutputShaderErrorMessage(ID3DBlob* errorMessage, std::string shaderFilename);

		bool						CompileShader(ID3D11Device* device, std::string fileName, shaderTarget target, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements);

	public:
									Shader();

		bool						Initialize(ID3D11Device* device, std::string vsFileName, std::string psFileName);

		bool						SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11Buffer* constants);

		bool						Render(ID3D11DeviceContext* deviceContext, UINT indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* shaderResource, ID3D11Buffer* constants);

		void						Shutdown();
	};
}

#endif // !__DX11_SHADER_HPP__