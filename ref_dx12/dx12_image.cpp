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
2017 Bleeding Eye Studios
*/

#include "dx12_local.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

std::shared_ptr<image_t> dx12::Image::Load(std::string name)
{
	if (name.length() < 5)
	{
		return nullptr;	//	ri.Sys_Error (ERR_DROP, "R_FindImage: bad name: %s", name);
	}

	// Create a new image
	std::shared_ptr<image_t> imgPtr = nullptr;

	Concurrency::parallel_for_each(images.begin(), images.end(), [&](std::shared_ptr<image_t> image)
	{
		if (name.compare(image->name) == 0)
		{
			// Found it
			imgPtr = image;
		}
	});

	if (imgPtr == nullptr)
	{
		// We didn't find it already, make a new one
		imgPtr = std::make_shared<image_t>();

		// We need this to get a compliant path string
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		
		// Determine the image type
		std::string extension = std::experimental::filesystem::path(name).extension().string();

		if (extension.compare(".pcx") == 0)
		{
			// Requesting a .pcx file
		}
		else if (extension.compare(".wal") == 0)
		{
			// Requesting a .wal file
		}
		else if (extension.compare(".tga") == 0)
		{
			// Requesting a .tga file
		}
		else if (extension.compare(".dds") == 0)
		{
			// Requesting a .dds file
			ComPtr<ID3D12Resource> tex;

			ResourceUploadBatch resourceUpload(ref->sys->d3dDevice);

			resourceUpload.Begin();

			DX::ThrowIfFailed(
				DirectX::CreateDDSTextureFromFile(ref->sys->d3dDevice,
												resourceUpload, 
												convert.from_bytes(imgPtr->name).c_str(), 
												images.at(imgPtr).ReleaseAndGetAddressOf())
			);

			// Upload the resources to the GPU.
			auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());

			// Wait for the upload thread to terminate
			uploadResourcesFinished.wait();
		}
		else if (extension.compare(".hdr") == 0)
		{
			// Requesting a .hdr file
		}
		else if (extension.compare(".exr") == 0)
		{
			// Requesting a .exr file
		}
		else
		{
			// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)
			ResourceUploadBatch resourceUpload(ref->sys->d3dDevice);

			resourceUpload.Begin();

			DX::ThrowIfFailed(
				DirectX::CreateWICTextureFromFile(ref->sys->d3dDevice,
												resourceUpload, 
												convert.from_bytes(imgPtr->name).c_str(),
												images.at(imgPtr).ReleaseAndGetAddressOf(),
												true)
			);

			// Upload the resources to the GPU.
			auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());

			// Wait for the upload thread to terminate
			uploadResourcesFinished.wait();
		}
	}

	// Return the pointer
	return imgPtr;
}