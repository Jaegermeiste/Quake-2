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

#ifndef __DX11_SYSTEM_HPP__
#define __DX11_SYSTEM_HPP__
#pragma once

#include "dx11_local.hpp"

namespace dx11
{
	class System {
	private:
		bool	inRegistration;
		bool	uploadBatchOpen;

	public:
		System();
		~System();

		void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

		ID3D11Device					*d3dDevice;
		ID3D11CommandQueue				*cmdQueue;

		DirectX::ResourceUploadBatch	*resourceUpload;

		void	BeginRegistration();
		void	EndRegistration();

		void	BeginUpload();
		void	EndUpload();
	};

	extern System* sys;
}

#endif // !__DX11_SYSTEM_HPP__