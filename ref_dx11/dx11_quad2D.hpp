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

#ifndef __DX11_QUAD2D_HPP__
#define __DX11_QUAD2D_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	__declspec(align(16)) class Quad2D {
		friend class System;
		friend class SubsystemText;
		friend class Subsystem2D;
		friend class Draw;
	private:
		ID3D11DeviceContext*		m_context = nullptr;
		ID3D11Buffer*				m_vertexBuffer = nullptr;
		ID3D11Buffer*				m_indexBuffer = nullptr;
		
		unsigned int				m_vertexCount = 0,
									m_indexCount = 0;

		int							m_xPrev = -1,
									m_yPrev = -1,
									m_widthPrev = -1,
									m_heightPrev = -1;

		float						m_u1Prev = -1,
									m_v1Prev = -1,
									m_u2Prev = -1,
									m_v2Prev = -1;

		byte						m_padding[12];

		DirectX::XMVECTORF32		m_colorPrev = DirectX::Colors::White;

		bool						InitializeBuffers();

		bool						UpdateBuffers(int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color);

		void						RenderBuffers() const;

	public:
									Quad2D();

		bool						Initialize(ID3D11DeviceContext* context, int x, int y, int width, int height, DirectX::XMVECTORF32 color);

		void						Render(int x, int y, int width, int height, DirectX::XMVECTORF32 color);

		void						Render(int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color);

		unsigned int				IndexCount() { return m_indexCount; };

		void						Shutdown();

		//https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
		void* operator new(size_t i)
		{
			return _mm_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_mm_free(p);
		}
	};
}

#endif // !__DX11_QUAD2D_HPP__