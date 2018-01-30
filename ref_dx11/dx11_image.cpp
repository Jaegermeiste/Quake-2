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

	// Destroy all the loaded images
	for (auto & image : m_images)
	{
		SAFE_RELEASE(image.second->m_shaderResourceView);
		SAFE_RELEASE(image.second->m_texture2D);
		SAFE_RELEASE(image.second->m_resource);
		ZeroMemory(&image.second->m_textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		image.second->m_registrationSequence = 0;
		image.second->m_name.clear();
		image.second->m_format.clear();
	}

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

	byte			*raw	= nullptr,
					*pic	= nullptr,
					*pal	= nullptr;
	unsigned int	width	= 0, 
					height	= 0;

	//
	// load the file
	//
	int len = ref->client->FS_LoadFile("pics/colormap.pcx", (void **)&raw);
	if (!raw)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't load pics/colormap.pcx");
		return;
	}

	// get the palette
	LOG(info) << "Loading pics/colormap.pcx";
	LoadPCX(raw, len, &pic, &pal, width, height);

	if (!pal)
	{
		ref->client->Sys_Error(ERR_FATAL, "Couldn't load pics/colormap.pcx");
		return;
	}

	for (unsigned int i = 0; i < 256; i++)
	{
		m_8to32table[i].r = pal[i * 3 + 0];
		m_8to32table[i].g = pal[i * 3 + 1];
		m_8to32table[i].b = pal[i * 3 + 2];
		m_8to32table[i].a = 255;

		//unsigned int v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
		//m_8to32table[i] = LittleULong(v);
	}

	m_8to32table[255].c &= LittleLong(0);	// 255 is transparent

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
void dx11::ImageManager::LoadPCX(byte* raw, int len, byte **pic, byte **palette, unsigned int &width, unsigned int &height)
{
	LOG_FUNC();

	pcx_t	*pcx		= nullptr;
	byte	dataByte	= 0;
	int		runLength	= 0;
	byte	*out		= nullptr,
			*pix		= nullptr;

	*pic = nullptr;
	*palette = nullptr;
	width = 0;
	height = 0;

	if ((!raw) || (len <= 0))
	{
		LOG(warning) << "Empty buffer passed.";
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
		ref->client->Con_Printf(PRINT_ALL, "Bad PCX file.");
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
		ref->client->Con_Printf(PRINT_DEVELOPER, "PCX file was malformed.");
		delete[] *pic;
		*pic = nullptr;
	}
}

std::shared_ptr<dx11::Texture2D> dx11::ImageManager::CreateTexture2DFromRaw(ID3D11Device* m_d3dDevice, std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> texture = nullptr;
	D3D11_SUBRESOURCE_DATA		data;
	HRESULT hr = E_UNEXPECTED;

	if (raw != nullptr)
	{
		texture = std::make_shared<dx11::Texture2D>();

		ZeroMemory(&texture->m_textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));

		texture->m_name = name;
		texture->m_textureDesc.Width = width;
		texture->m_textureDesc.Height = height;
		if (generateMipmaps)
		{
			texture->m_textureDesc.MipLevels = 0;
			texture->m_textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		else
		{
			texture->m_textureDesc.MipLevels = 1;
			texture->m_textureDesc.MiscFlags = 0;
		}
		texture->m_textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture->m_textureDesc.SampleDesc.Count = 1;
		texture->m_textureDesc.SampleDesc.Quality = static_cast<UINT>(D3D11_STANDARD_MULTISAMPLE_PATTERN);
		texture->m_textureDesc.Usage = D3D11_USAGE_DEFAULT;
		texture->m_textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture->m_textureDesc.CPUAccessFlags = 0;
		texture->m_textureDesc.ArraySize = 1;
		data.SysMemPitch = width * (sizeof(unsigned int) / sizeof(byte));
		data.SysMemSlicePitch = width * height * (sizeof(unsigned int) / sizeof(byte));
		unsigned int* rgba32 = new unsigned int[width * height]();

		if (bpp == BPP_8)
		{
			DirectX::PackedVector::XMCOLOR *d8to32table = m_8to32table;	// Hack around the lambda function parameter issue

			// De-palletize the texture data

			//for (unsigned int i = 0; i < (width * height); i++)
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32, &d8to32table](unsigned int i)
			{
				if (raw[i] == 255)
				{
					// Transparent
					rgba32[i] = 0x00000000;
				}
				else
				{
					// Paletted
					rgba32[i] = d8to32table[raw[i]];
					//rgba32[i] = (d8to24table[raw[i]] << 24u | 255u);
					//((byte *)&rgba32[i])[0] = ((byte *)&d8to24table[raw[i]])[0];
					//((byte *)&rgba32[i])[1] = ((byte *)&d8to24table[raw[i]])[1];
					//((byte *)&rgba32[i])[2] = ((byte *)&d8to24table[raw[i]])[2];
					//((byte *)&rgba32[i])[3] = 255;
				}
			});
		}
		else if (bpp == BPP_24)
		{
			// 24 bpp
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32](unsigned int i)
			{
				unsigned int index = i * 3;
				rgba32[i] = (raw[index] << 24u | raw[index + 1] << 16u | raw[index + 2] << 8u | 255u);
			});
		}
		else if (bpp == BPP_32)
		{
			std::memcpy(&rgba32, raw, width * height * (sizeof(unsigned int) / sizeof(byte)));
		}

		data.pSysMem = rgba32;

		hr = m_d3dDevice->CreateTexture2D(&texture->m_textureDesc, &data, &texture->m_texture2D);
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

inline void dx11::ImageManager::UploadScratchImage(ScratchImage &scratch, ID3D11Resource** pResource, bool generateMipMap)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	unsigned int miscFlags = 0;

	if (generateMipMap)
	{
		miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	hr = CreateTextureEx(ref->sys->dx->m_d3dDevice, scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(), 
							D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, false, pResource);

	if (FAILED(hr))
	{
		ref->client->Con_Printf(PRINT_ALL, "Failed to create resource for ScratchImage.");
	}
}

std::shared_ptr<dx11::Texture2D> dx11::ImageManager::Load(std::string name, imagetype_t type)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	byte	*buffer = nullptr;
	int bufferSize = -1;
	bool generateMipMap = ((type != it_pic) && (type != it_sky));
	unsigned int	miscFlags = 0;

	if (name.length() < 5)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "Bad name (<5): " + name);
	}

	// First, see if the image has already been loaded in the map:
	if (m_images.count(name) > 0)
	{
		return m_images[name];
	}

	// Create a new image
	if (m_images[name] == nullptr)
	{
		// We didn't find it already, make a new one
		m_images[name] = std::make_shared<Texture2D>();

		// Determine the image type
		std::string fileName = std::experimental::filesystem::path(name).filename().string();
		std::string extension = std::experimental::filesystem::path(name).extension().string();
		std::string path = "";

		if ((fileName.at(0) != '/') && (fileName.at(0) != '\\'))
		{
			switch (type)
			{
			case it_pic:
				path = "pics/";
				break;
			case it_skin:
			case it_sprite:
			case it_wall:
			case it_sky:
			default:
				break;
			}
		}
		else
		{
			fileName = name.substr(1, name.length() - 1);
		}

		// Iterate through all possible format extensions until we find the file
		for (auto & format : m_imageExtensions)
		{
			buffer = nullptr;
			bufferSize = -1;

			// Attempt to load the file
			bufferSize = ref->client->FS_LoadFile(path + fileName + "." + format, (void **)&buffer);

			if ((bufferSize > 0) && (buffer))
			{
				m_images[name]->m_format = format;
				break;
			}
		}

		if (generateMipMap)
		{
			miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		//ref->sys->BeginUpload();

		if (m_images[name]->m_format.compare("pcx") == 0)
		{
			byte	*pic = nullptr, *palette = nullptr;
			LoadPCX(buffer, bufferSize, &pic, &palette, m_images[name]->m_textureDesc.Width, m_images[name]->m_textureDesc.Height);
			m_images[name] = CreateTexture2DFromRaw(ref->sys->dx->m_d3dDevice, name, m_images[name]->m_textureDesc.Width, m_images[name]->m_textureDesc.Height, false, 8, pic);
		}
		else if (m_images[name]->m_format.compare("wal") == 0)
		{
			// Requesting a .wal file
		}
		else if (m_images[name]->m_format.compare("tga") == 0)
		{
			// Requesting a .tga file
			ScratchImage scratch;
			TexMetadata info;
			hr = LoadFromTGAMemory(buffer, static_cast<size_t>(bufferSize), &info, scratch);

			UploadScratchImage(scratch, &m_images[name]->m_resource, generateMipMap);

		}
		else if (m_images[name]->m_format.compare("dds") == 0)
		{
			// .dds file
			hr = CreateDDSTextureFromMemoryEx(ref->sys->dx->m_d3dDevice,
												ref->sys->dx->m_immediateContext,
												static_cast<uint8_t*>(buffer), static_cast<size_t>(bufferSize),
												0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
												NULL, miscFlags,
												false,
												&m_images[name]->m_resource, &m_images[name]->m_shaderResourceView,
												nullptr);

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with DDS loader.");
			}

		}
		else if (m_images[name]->m_format.compare("hdr") == 0)
		{
			// Requesting a .hdr file
			TexMetadata metadata;
			hr = GetMetadataFromHDRMemory(buffer, bufferSize, metadata);

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to extract HDR metadata from " + name + ".");
			}
			else
			{
				ScratchImage scratch;
				hr = LoadFromHDRMemory(buffer, bufferSize, &metadata, scratch);

				if (FAILED(hr))
				{
					ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with HDR loader.");
				}
				else
				{
					// Upload scratch
					UploadScratchImage(scratch, &m_images[name]->m_resource, generateMipMap);
				}
			}
		}
		else if (m_images[name]->m_format.compare("exr") == 0)
		{
			// Requesting a .exr file
		}
		else
		{
			// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)
			hr = CreateWICTextureFromMemoryEx(ref->sys->dx->m_d3dDevice, 
												ref->sys->dx->m_immediateContext, 
												static_cast<uint8_t*>(buffer), static_cast<size_t>(bufferSize),
												0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
												NULL, miscFlags,
												WIC_LOADER_DEFAULT, 
												&m_images[name]->m_resource, &m_images[name]->m_shaderResourceView);

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with WIC loader.");
			}
		}

		ref->sys->EndUpload();
	}

	if(buffer)
	{
		ref->client->FS_FreeFile(buffer);
	}

	// Get texture/resource as necessary
	if ((m_images[name]->m_resource) && (!m_images[name]->m_texture2D))
	{
		hr = m_images[name]->m_resource->QueryInterface(IID_ID3D11Texture2D, (void **)&m_images[name]->m_texture2D);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get Texture2D from resource.");
		}
	}
	else if ((!m_images[name]->m_resource) && (m_images[name]->m_texture2D))
	{
		hr = m_images[name]->m_texture2D->QueryInterface(IID_ID3D11Resource, (void **)&m_images[name]->m_resource);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get resource from Texture2D.");
		}
	}

	// Get SRV as necessary
	if ((m_images[name]->m_resource) && (!m_images[name]->m_shaderResourceView))
	{
		hr = ref->sys->dx->m_d3dDevice->CreateShaderResourceView(m_images[name]->m_resource, NULL, &m_images[name]->m_shaderResourceView);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get ShaderResourceView from resource.");
		}
	}

	// Overwrite the texture desc with whatever is in memory/on GPU
	if (m_images[name]->m_texture2D)
	{
		m_images[name]->m_texture2D->GetDesc(&m_images[name]->m_textureDesc);
	}

	m_images[name]->m_name = name;
	m_images[name]->m_imageType = type;

	// Return the pointer
	return m_images[name];
}