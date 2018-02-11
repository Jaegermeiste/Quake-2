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

#ifndef __DX11_SUBSYSTEMTEXT_HPP__
#define __DX11_SUBSYSTEMTEXT_HPP__
#pragma once

#include "dx11_local.hpp"

#define SMALL_CHAR_SIZE	8
#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	8
#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define MAX_BATCH_CHARS		1024
#define MAX_VERTICES		MAX_BATCH_CHARS * 4		// 4 corners to a quad
#define MAX_INDICES			MAX_BATCH_CHARS * 6		// Each tris references 3 corners, 2 tris to a quad = 6 indices

namespace dx11
{
	__declspec(align(16)) class SubsystemText {
		friend class System;
	private:
#ifdef USE_DIRECT2D
		IDWriteFactory*					m_writeFactory = nullptr;
		IDWriteTextFormat*				m_textFormat = nullptr;
#endif
		ID3D11DeviceContext*			m_context = nullptr;
		ID3D11Buffer*					m_vertexBuffer = nullptr;
		ID3D11Buffer*					m_indexBuffer = nullptr;

		dx11::Vertex2D*					m_vertices = nullptr;
		unsigned long*					m_indices = nullptr;

		unsigned int					m_vertexCount = 0,
										m_indexCount = 0;

		bool							m_inBatch;
		
		byte							m_padding[3];

		std::shared_ptr<Shader>			m_shader = nullptr;

		byte							m_padding2[8];

		bool							InitializeBuffers();

	public:
										SubsystemText();

		bool							Initialize();

#ifdef USE_DIRECT2D
		void							RenderText(int x, int y, int w, int h, std::string text, ID2D1SolidColorBrush* colorBrush);
		void							RenderText(int x, int y, int w, int h, WCHAR* text, ID2D1SolidColorBrush* colorBrush);
#endif

		void							DrawSmallChar		(int x, int y, int ch);

		void							DrawSmallStringExt	(int x, int y, std::string string, const DirectX::XMVECTORF32 setColor, bool forceColor);

		void							DrawBigChar			(int x, int y, int ch);

		void							DrawBigStringExt	(int x, int y, std::string string, const DirectX::XMVECTORF32 setColor, bool forceColor);

		void							Flush();

		void							Shutdown();

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX11_SUBSYSTEMTEXT_HPP__