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

#ifndef __DX12_LIGHT_HPP__
#define __DX12_LIGHT_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12
{
	class Light
	{
	private:
		static float CalculateAttenuationDistance(const dx12::Light* light)
		{
			// Define a threshold for minimum acceptable intensity
			const float MinIntensity = 0.01f;

			// Calculate the maximum distance where the light's intensity is above the threshold
			// Using the attenuation formula: I = I0 / (constant + linear * d + quadratic * d^2)
			// We solve for d where I >= MinIntensity * I0
			float maxDistance = (-light->linearAttenuation +
				sqrtf(light->linearAttenuation * light->linearAttenuation - 4 * light->quadraticAttenuation * (light->constantAttenuation - 1.0f / MinIntensity))) /
				(2 * light->quadraticAttenuation);

			return maxDistance;
		};

	public:
		XMVECTOR	        origin;
		XMVECTOR	        color;
		XMVECTOR	        angles;
		XMVECTOR	        speed;
		XMVECTOR	        flareOrigin;
		float				cone;
		float				flareSize;
		float				fog;
		float				fogDensity;
		int					style;
		int					filter;
		int					flags;
		bool				flare;
		bool				shadowCaster;
		bool				ambient;
		
		float               constantAttenuation = 1.0f;
		float               linearAttenuation = 0.09f;
		float               quadraticAttenuation = 0.032f;

		float               radius = 0.0f;

		Light(XMVECTOR origin, float radius, XMVECTOR color = DirectX::Colors::White)
		{
			style			= 0;
			filter		= 0;
			shadowCaster	= true;
			ambient		= false;
			cone			= 0.0f;
			flareSize		= 0;
			flare			= false;
			flags			= NULL;
			fog			= 0.0f;
			fogDensity	= 0.0f;
			angles		= DirectX::XMVectorZero();
			speed			= DirectX::XMVectorZero();
			this->origin        = origin;
			this->color = color;
			flareOrigin	= DirectX::XMVectorZero();
			constantAttenuation = 1.0f;
			linearAttenuation = 4.5f / radius;
			quadraticAttenuation = 75.0f / (radius * radius);
			this->radius = CalculateAttenuationDistance(this);
		};

		Light(XMFLOAT3 origin, float radius, XMVECTOR color = DirectX::Colors::White)
		{
			style = 0;
			filter = 0;
			shadowCaster = true;
			ambient = false;
			cone = 0.0f;
			flareSize = 0;
			flare = false;
			flags = NULL;
			fog = 0.0f;
			fogDensity = 0.0f;
			angles = DirectX::XMVectorZero();
			speed = DirectX::XMVectorZero();
			this->origin = { origin.x, origin.y, origin.z };
			this->color = color;
			flareOrigin = DirectX::XMVectorZero();
			constantAttenuation = 1.0f;
			linearAttenuation = 4.5f / radius;
			quadraticAttenuation = 75.0f / (radius * radius);
			this->radius = CalculateAttenuationDistance(this);
		};

		Light(float origin[3], float radius, XMVECTOR color = DirectX::Colors::White)
		{
			style = 0;
			filter = 0;
			shadowCaster = true;
			ambient = false;
			cone = 0.0f;
			flareSize = 0;
			flare = false;
			flags = NULL;
			fog = 0.0f;
			fogDensity = 0.0f;
			angles = DirectX::XMVectorZero();
			speed = DirectX::XMVectorZero();
			this->origin = { origin[0], origin[1], origin[2] };
			this->color = color;
			flareOrigin = DirectX::XMVectorZero();
			constantAttenuation = 1.0f;
			linearAttenuation = 4.5f / radius;
			quadraticAttenuation = 75.0f / (radius * radius);
			this->radius = CalculateAttenuationDistance(this);
		};

		Light(vec3_t origin, float radius, vec3_t color)
		{
			style = 0;
			filter = 0;
			shadowCaster = true;
			ambient = false;
			cone = 0.0f;
			flareSize = 0;
			flare = false;
			flags = NULL;
			fog = 0.0f;
			fogDensity = 0.0f;
			angles = DirectX::XMVectorZero();
			speed = DirectX::XMVectorZero();
			this->origin = { origin[0], origin[1], origin[2] };
			this->color = { color[0], color[1], color[2], 1.0f };
			flareOrigin = DirectX::XMVectorZero();
			constantAttenuation = 1.0f;
			linearAttenuation = 4.5f / radius;
			quadraticAttenuation = 75.0f / (radius * radius);
			this->radius = CalculateAttenuationDistance(this);
		};
	};
}

#endif // !__DX12_LIGHT_HPP__