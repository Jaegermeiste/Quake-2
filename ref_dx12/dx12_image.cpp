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

/*
===============
GetPalette
===============
*/
void dx12::Image::GetPalette(void)
{
	byte			*pic	= nullptr,
					*pal	= nullptr;
	unsigned int	width	= 0, 
					height	= 0;

	// get the palette

	LoadPCX("pics/colormap.pcx", &pic, &pal, width, height);
	if (!pal)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't load pics/colormap.pcx");
	}

	for (unsigned int i = 0; i < 256; i++)
	{
		unsigned int r = pal[i * 3 + 0];
		unsigned int g = pal[i * 3 + 1];
		unsigned int b = pal[i * 3 + 2];

		unsigned int v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
		d_8to24table[i] = LittleLong(v);
	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	free(pic);
	free(pal);
}

/*
================
GL_LoadWal
================
*/
void dx12::Image::LoadWal(std::string fileName, byte **pic, unsigned int &width, unsigned int &height)
{
	miptex_t		*mt = nullptr;
	unsigned int	ofs	= 0;

	ref->client->FS_LoadFile(fileName, (void **)&mt);

	if (!mt)
	{
		ref->client->Con_Printf(PRINT_ALL, "GL_FindImage: can't load " + fileName + "\n");
		return;
	}

	width = LittleLong(mt->width);
	height = LittleLong(mt->height);
	ofs = LittleLong(mt->offsets[0]);

	image = GL_LoadPic(name, (byte *)mt + ofs, width, height, it_wall, 8);

	ref->client->FS_FreeFile((void *)mt);
}

/*
==============
LoadPCX
==============
*/
void dx12::Image::LoadPCX(std::string fileName, byte **pic, byte **palette, unsigned int &width, unsigned int &height)
{
	byte	*raw		= nullptr;
	pcx_t	*pcx		= nullptr;
	int		len			= 0;
	int		dataByte	= 0, 
			runLength	= 0;
	byte	*out		= nullptr,
			*pix		= nullptr;

	*pic = nullptr;
	*palette = nullptr;

	//
	// load the file
	//
	len = ref->client->FS_LoadFile(fileName, (void **)&raw);
	if (!raw)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "Bad pcx file " + fileName + "\n");
		return;
	}

	//
	// parse the PCX file
	//
	pcx = reinterpret_cast<pcx_t *>(raw);

	pcx->xmin = LittleShort(pcx->xmin);
	pcx->ymin = LittleShort(pcx->ymin);
	pcx->xmax = LittleShort(pcx->xmax);
	pcx->ymax = LittleShort(pcx->ymax);
	pcx->hres = LittleShort(pcx->hres);
	pcx->vres = LittleShort(pcx->vres);
	pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
	pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ref->client->Con_Printf(PRINT_ALL, "Bad pcx file " + fileName + "\n");
		return;
	}

	out = new byte[(pcx->ymax + 1) * (pcx->xmax + 1)];

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = new byte[768];
		memcpy(*palette, reinterpret_cast<byte *>(pcx) + len - 768, 768);
	}

	width = pcx->xmax + 1;
	height = pcx->ymax + 1;

	for (unsigned short y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1)
	{
		for (unsigned short x = 0; x <= pcx->xmax; )
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - reinterpret_cast<byte *>(pcx) > len)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "PCX file " + fileName + " was malformed");
		delete[] *pic;
		*pic = nullptr;
	}

	ref->client->FS_FreeFile(pcx);
}

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
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertToWide;
		
		// Determine the image type
		std::string extension = std::experimental::filesystem::path(name).extension().string();

		ComPtr<ID3D12Resource> tex;

		if (extension.compare(".pcx") == 0)
		{
			// Requesting a .pcx file
			std::unique_ptr<uint8_t[]> decodedData;
			D3D12_SUBRESOURCE_DATA subresource;
			DX::ThrowIfFailed(
				LoadWICTextureFromFile(ref->sys->d3dDevice, 
						convertToWide.from_bytes(imgPtr->name).c_str(), 
						tex.ReleaseAndGetAddressOf(),
						decodedData, 
						subresource));

			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(tex.Get(), 0, 1);

			// Create the GPU upload buffer.
			ComPtr<ID3D12Resource> uploadHeap;
			DX::ThrowIfFailed(
				ref->sys->d3dDevice->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(uploadHeap.GetAddressOf())));

			UpdateSubresources(commandList, tex.Get(), uploadHeap.Get(), 0, 0, 1, &subresource);
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

			DX::ThrowIfFailed(commandList->Close());
			m_deviceResources->GetCommandQueue()->ExecuteCommandLists(1,
				CommandListCast(&commandList));
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
			ResourceUploadBatch resourceUpload(ref->sys->d3dDevice);

			resourceUpload.Begin();

			DX::ThrowIfFailed(
				DirectX::CreateDDSTextureFromFile(ref->sys->d3dDevice,
												resourceUpload, 
												convertToWide.from_bytes(imgPtr->name).c_str(), 
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
												convertToWide.from_bytes(imgPtr->name).c_str(),
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