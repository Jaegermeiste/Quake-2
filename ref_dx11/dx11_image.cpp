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

#include "dx11_local.hpp"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

bool dx11::ImageManager::Initialize()
{
	LOG_FUNC();

	GetPalette();

	return true;
}

void dx11::ImageManager::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	LOG(info) << "Shutdown complete.";
}

/*
===============
GetPalette
===============
*/
void dx11::ImageManager::GetPalette(void)
{
	LOG_FUNC();

	byte			*pic	= nullptr,
					*pal	= nullptr;
	unsigned int	width	= 0, 
					height	= 0;

	// get the palette
	LOG(info) << "Loading pics/colormap.pcx";
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
		m_8to24table[i] = LittleULong(v);
	}

	m_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	if (pic != nullptr)
	{
		delete[] pic;
		pic = nullptr;
	}

	if (pal != nullptr)
	{
		delete[] pal;
		pal = nullptr;
	}
}

/*
================
LoadWal
================
*/
void dx11::ImageManager::LoadWal(std::string fileName, byte **pic, unsigned int &width, unsigned int &height)
{
	LOG_FUNC();

	miptex_t		*mt = nullptr;
	unsigned int	ofs	= 0;

	ref->client->FS_LoadFile(fileName, (void **)&mt);

	if (!mt)
	{
		ref->client->Con_Printf(PRINT_ALL, "GL_FindImage: can't load " + fileName + "\n");
		return;
	}

	width = LittleULong(mt->width);
	height = LittleULong(mt->height);
	ofs = LittleULong(mt->offsets[0]);

	//image = GL_LoadPic(name, (byte *)mt + ofs, m_width, m_height, it_wall, 8);

	ref->client->FS_FreeFile((void *)mt);
}

/*
==============
LoadPCX
==============
*/
void dx11::ImageManager::LoadPCX(std::string fileName, byte **pic, byte **palette, unsigned int &width, unsigned int &height)
{
	LOG_FUNC();

	byte	*raw		= nullptr;
	pcx_t	*pcx		= nullptr;
	int		len			= 0;
	byte	dataByte	= 0;
	int		runLength	= 0;
	byte	*out		= nullptr,
			*pix		= nullptr;

	*pic = nullptr;
	*palette = nullptr;

	//
	// load the file
	//
	len = ref->client->FS_LoadFile(fileName, (void **)&raw);
	if ((len < 0) || (!raw))
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "Bad PCX file " + fileName);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = reinterpret_cast<pcx_t *>(raw);

	pcx->xmin = LittleUShort(pcx->xmin);
	pcx->ymin = LittleUShort(pcx->ymin);
	pcx->xmax = LittleUShort(pcx->xmax);
	pcx->ymax = LittleUShort(pcx->ymax);
	pcx->hres = LittleUShort(pcx->hres);
	pcx->vres = LittleUShort(pcx->vres);
	pcx->bytes_per_line = LittleUShort(pcx->bytes_per_line);
	pcx->palette_type = LittleUShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ref->client->Con_Printf(PRINT_ALL, "Bad PCX file " + fileName);
		return;
	}

	out = new byte[(pcx->ymax + 1u) * (pcx->xmax + 1u)]();

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = new byte[768]();
		memcpy(*palette, reinterpret_cast<byte *>(pcx) + len - 768, 768);
	}

	width = msl::utilities::SafeInt<unsigned int>(pcx->xmax + 1);
	height = msl::utilities::SafeInt<unsigned int>(pcx->ymax + 1);

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

dx11::Texture2D* dx11::ImageManager::CreateTexture2DFromRaw(ID3D11Device* m_d3dDevice, std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte** raw)
{
	LOG_FUNC();

	dx11::Texture2D* texture = nullptr;
	HRESULT hr = E_UNEXPECTED;

	if ((*raw) != nullptr)
	{
		texture = new dx11::Texture2D;

		ZeroMemory(&texture->m_textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		ZeroMemory(&texture->m_data, sizeof(D3D11_SUBRESOURCE_DATA));

		texture->m_name = name;
		texture->m_textureDesc.Width = width;
		texture->m_textureDesc.Height = height;
		if (generateMipmaps)
		{
			texture->m_textureDesc.MipLevels = 0;
		}
		else
		{
			texture->m_textureDesc.MipLevels = 1;
		}
		texture->m_textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
		texture->m_textureDesc.SampleDesc.Count = 1;
		texture->m_textureDesc.SampleDesc.Quality = static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN);
		texture->m_textureDesc.Usage = D3D11_USAGE_DEFAULT;
		texture->m_textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture->m_textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texture->m_textureDesc.MiscFlags = 0;
		texture->m_data.SysMemPitch = width * (sizeof(unsigned int) / sizeof(byte));
		texture->m_data.SysMemSlicePitch = width * height * (sizeof(unsigned int) / sizeof(byte));
		unsigned int* rgba32 = new unsigned int[width * height]();

		if (bpp == BPP_8)
		{
			unsigned int *d8to24table = m_8to24table;	// Hack around the lambda function parameter issue

			// De-palletize the texture data

			//for (unsigned int i = 0; i < (m_width * m_height); i++)
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32, &d8to24table](unsigned int i)
			{
				if (*raw[i] == 255)
				{
					// Transparent
					rgba32[i] = 0;
				}
				else
				{
					// Paletted
					rgba32[i] = d8to24table[*raw[i]];
				}
			});
		}
		else if (bpp == BPP_24)
		{
			// 24 bpp
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32](unsigned int i)
			{
				unsigned int index = i * 3;
				rgba32[i] = (*raw[index] << 24u | *raw[index + 1] << 16u | *raw[index + 2] << 8u | 255u);
			});
		}
		else if (bpp == BPP_32)
		{
			std::memcpy(&rgba32, *raw, width * height * (sizeof(unsigned int) / sizeof(byte)));
		}

		texture->m_data.pSysMem = rgba32;

		hr = m_d3dDevice->CreateTexture2D(&texture->m_textureDesc, &texture->m_data, &texture->m_texture2D);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create texture";
			delete[] rgba32;
			rgba32 = nullptr;
			return nullptr;
		}
	}

	return texture;
}

void dx11::ImageManager::UploadScratchImage(ScratchImage &image, ID3D11Resource** pResource, bool generateMipMap)
{
	LOG_FUNC();

	/*DX::ThrowIfFailed(
		CreateTexture(ref->sys->d3dDevice, image.GetMetadata(), pResource)
	);

	D3D11_SUBRESOURCE_DATA srData;
	size_t rowPitch = 0;
	size_t slicePitch = 0;
	ComputePitch(image.GetMetadata().format, image.GetMetadata().m_width, image.GetMetadata().m_height, rowPitch, slicePitch);
	srData.RowPitch = rowPitch;
	srData.SlicePitch = slicePitch;
	srData.pData = image.GetPixels();

	ref->sys->resourceUpload->Upload(*pResource, 0, &srData, 1);

	ref->sys->resourceUpload->Transition(*pResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	if (generateMipMap)
	{
		ref->sys->resourceUpload->GenerateMips(*pResource);
	}*/
}

std::shared_ptr<image_t> dx11::ImageManager::Load(std::string name, imagetype_t type)
{
	LOG_FUNC();

	if (name.length() < 5)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "Bad name (<5): " + name);
	}

	// Create a new image
	std::shared_ptr<image_t> imgPtr = nullptr;

	/*Concurrency::parallel_for_each(images.begin(), images.end(), [&](std::shared_ptr<image_t> image)
	{
		if (name.compare(image->name) == 0)
		{
			// Found it
			imgPtr = image;
		}
	});*/

	if (imgPtr == nullptr)
	{
		// We didn't find it already, make a new one
		imgPtr = std::make_shared<image_t>();

		// Determine the image type
		std::string extension = std::experimental::filesystem::path(name).extension().string();

		bool generateMipMap = ((type != it_pic) && (type != it_sky));

		ref->sys->BeginUpload();

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
			byte	*buffer = nullptr;
			int bufferSize = ref->client->FS_LoadFile(name, (void **)&buffer);
			if ((bufferSize < 0) || (!buffer))
			{
				ref->client->Con_Printf(PRINT_ALL, "Bad TGA file: " + name + "\n");
				return nullptr;
			}

			ScratchImage image;
			TexMetadata info;
			DX::ThrowIfFailed(
				LoadFromTGAMemory(buffer, static_cast<size_t>(bufferSize), &info, image)
			);

			UploadScratchImage(image, 
							m_images.at(imgPtr).ReleaseAndGetAddressOf(), 
							generateMipMap);

			ref->client->FS_FreeFile(buffer);
		}
		else if (extension.compare(".dds") == 0)
		{
			// Requesting a .dds file
			/*DX::ThrowIfFailed(
				CreateDDSTextureFromFile(ref->sys->d3dDevice,
										*(ref->sys->resourceUpload), 
										ref->sys->convertUTF.from_bytes(imgPtr->name).c_str(), 
										images.at(imgPtr).ReleaseAndGetAddressOf(),
										generateMipMap)
			);*/

		}
		else if (extension.compare(".hdr") == 0)
		{
			// Requesting a .hdr file
			ScratchImage image;
			DX::ThrowIfFailed(
				LoadFromHDRFile(ref->sys->convertUTF.from_bytes(imgPtr->name).c_str(), nullptr, image)
			);

			UploadScratchImage(image,
				m_images.at(imgPtr).ReleaseAndGetAddressOf(),
				generateMipMap);
		}
		else if (extension.compare(".exr") == 0)
		{
			// Requesting a .exr file
		}
		else
		{
			// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)
			/*DX::ThrowIfFailed(
				CreateWICTextureFromFile(ref->sys->d3dDevice,
										*(ref->sys->resourceUpload),
										ref->sys->convertUTF.from_bytes(imgPtr->name).c_str(),
										images.at(imgPtr).ReleaseAndGetAddressOf(),
										generateMipMap)
			);*/
		}

		ref->sys->EndUpload();
	}

	// Return the pointer
	return imgPtr;
}