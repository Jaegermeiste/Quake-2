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

#ifndef __DX11_LIGHT_HPP__
#define __DX11_LIGHT_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class Light
	{
	public:
		DirectX::XMVECTOR	m_origin;
		DirectX::XMVECTOR	m_radius;
		DirectX::XMVECTOR	m_color;
		DirectX::XMVECTOR	m_angles;
		DirectX::XMVECTOR	m_speed;
		DirectX::XMVECTOR	m_flareOrigin;
		float				m_cone;
		float				m_flareSize;
		float				m_fog;
		float				m_fogDensity;
		int					m_style;
		int					m_filter;
		int					m_flags;
		bool				m_flare;
		bool				m_shadowCaster;
		bool				m_ambient;
		byte				m_padding;

		Light()
		{
			m_style			= 0;
			m_filter		= 0;
			m_shadowCaster	= true;
			m_ambient		= false;
			m_cone			= 0.0f;
			m_flareSize		= 0;
			m_flare			= false;
			m_flags			= NULL;
			m_fog			= 0.0f;
			m_fogDensity	= 0.0f;
			m_radius		= DirectX::XMVectorZero();
			m_angles		= DirectX::XMVectorZero();
			m_speed			= DirectX::XMVectorZero();
			m_origin		= DirectX::XMVectorZero();
			m_color			= DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);	// White Light by default
			m_flareOrigin	= DirectX::XMVectorZero();
		};
	};
}

#endif // !__DX11_LIGHT_HPP__