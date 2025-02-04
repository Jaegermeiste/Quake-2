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

#ifndef __DX12_QUAD2D_HPP__
#define __DX12_QUAD2D_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	__declspec(align(16)) class Quad2D: public IndexedGeometry {
		friend class System;
		friend class SubsystemText;
		friend class Subsystem2D;
		friend class Draw;
	private:
		int							m_xPrev = -1,
									m_yPrev = -1,
									m_widthPrev = -1,
									m_heightPrev = -1;

		float						m_u1Prev = -1,
									m_v1Prev = -1,
									m_u2Prev = -1,
									m_v2Prev = -1;

		DirectX::XMVECTORF32		m_colorPrev = DirectX::Colors::White;

		bool						InitializeBuffers() override;
		bool						InitializeBuffers(int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color);

		bool						UpdateBuffers(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color);

	public:
									Quad2D();

		bool						Initialize(int x, int y, int width, int height, DirectX::XMVECTORF32 color);

		using IndexedGeometry::Render;
		void						Render(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, DirectX::XMVECTORF32 color);
		void						Render(std::shared_ptr<CommandList> commandList, int x, int y, int width, int height, float u1, float v1, float u2, float v2, DirectX::XMVECTORF32 color);

		ALIGNED_16_MEMORY_OPERATORS;
	};
}

#endif // !__DX12_QUAD2D_HPP__