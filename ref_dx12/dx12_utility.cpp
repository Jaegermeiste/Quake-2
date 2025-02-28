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

namespace dx12_utility 
{
	boost::uuids::time_generator_v7 uuid7_gen;
};

Vector2 GetNormalizedDeviceCoordinates(int px, int py, int windowWidth, int windowHeight)
{
	Vector2 v = {};

	// Convert pixel coordinates to NDC (-1 to 1 range)
	v.x = (2.0f * px / windowWidth) - 1.0f;   // Convert x
	v.y = 1.0f - (2.0f * py / windowHeight);  // Convert y

	return v;
}

Vector4 GetNormalizedDeviceRectangle(int px, int py, int pw, int ph, int windowWidth, int windowHeight)
{
	Vector4 v = {};

	// Convert pixel coordinates to NDC (-1 to 1 range)
	v.x = (2.0f * px / windowWidth) - 1.0f;
	v.y = 1.0f - (2.0f * py / windowHeight);
	v.z = 2.0f * pw / windowWidth;
	v.w = 2.0f * ph / windowHeight;

	return v;
}