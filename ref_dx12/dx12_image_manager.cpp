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

#include "dx12_local.hpp"

bool dx12::ImageManager::Initialize()
{
	LOG_FUNC();

	GetPalette();

	m_conChars = Load("pics/conchars.pcx", it_pic);

	return true;
}

void dx12::ImageManager::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	if (m_rawTexture)
	{
		SAFE_RELEASE(m_rawTexture->m_resource);
		ZeroMemory(&m_rawTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
		m_rawTexture->m_format.clear();
		m_rawTexture->m_registrationSequence = 0;
		m_rawTexture = nullptr;
	}

	LOG(info) << "Shutdown complete.";
	}

/*
===============
GetPalette
===============
*/
void dx12::ImageManager::GetPalette(void)
{
	LOG_FUNC();

	byte			*raw = nullptr,
		*pic = nullptr,
		*pal = nullptr;
	unsigned int	width = 0,
		height = 0;

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

	SetRawPalette(nullptr);
}

/*
std::shared_ptr<dx12::Texture2D> dx12::ImageManager::CreateTexture2DFromRaw(std::string name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, DirectX::PackedVector::XMCOLOR *palette, D3D12_USAGE usage)
{
	LOG_FUNC();

	std::shared_ptr<dx12::Texture2D> texture = nullptr;
	D3D12_SUBRESOURCE_DATA		data;
	HRESULT hr = E_UNEXPECTED;

	if (raw != nullptr)
	{
		texture = std::make_shared<dx12::Texture2D>();

		ZeroMemory(&texture->m_textureDesc, sizeof(D3D12_TEXTURE2D_DESC));
		ZeroMemory(&data, sizeof(D3D12_SUBRESOURCE_DATA));

		texture->m_name = name;
		texture->m_textureDesc.Width = width;
		texture->m_textureDesc.Height = height;
		if (generateMipmaps)
		{
			texture->m_textureDesc.MipLevels = 0;
			texture->m_textureDesc.MiscFlags = D3D12_RESOURCE_MISC_GENERATE_MIPS;
		}
		else
		{
			texture->m_textureDesc.MipLevels = 1;
			texture->m_textureDesc.MiscFlags = 0;
		}
		texture->m_textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture->m_textureDesc.SampleDesc.Count = 1;
		texture->m_textureDesc.SampleDesc.Quality = static_cast<UINT>(D3D12_STANDARD_MULTISAMPLE_PATTERN);
		texture->m_textureDesc.Usage = usage;
		texture->m_textureDesc.BindFlags = D3D12_BIND_SHADER_RESOURCE;

		if (usage == D3D12_USAGE_DYNAMIC)
		{
			texture->m_textureDesc.CPUAccessFlags = D3D12_CPU_ACCESS_WRITE;
		}
		else
		{
			texture->m_textureDesc.CPUAccessFlags = 0;
		}

		texture->m_textureDesc.ArraySize = 1;
		data.SysMemPitch = width * (sizeof(unsigned int) / sizeof(byte));
		data.SysMemSlicePitch = width * height * (sizeof(unsigned int) / sizeof(byte));
		unsigned int* rgba32 = nullptr;

		if (bpp == BPP_8)
		{
			rgba32 = new unsigned int[width * height]();
			// De-palletize the texture data

			//for (unsigned int i = 0; i < (width * height); i++)
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32, &palette](unsigned int i)
			{
				if (raw[i] == 255)
				{
					// Transparent
					rgba32[i] = 0x00000000;
				}
				else
				{
					// Paletted
					rgba32[i] = palette[raw[i]];
				}
			});
		}
		else if (bpp == BPP_24)
		{
			rgba32 = new unsigned int[width * height]();

			// 24 bpp
			Concurrency::parallel_for(0u, (width * height), [&raw, &rgba32](unsigned int i)
			{
				unsigned int index = i * 3;
				rgba32[i] = (raw[index] << 24u | raw[index + 1] << 16u | raw[index + 2] << 8u | 255u);
			});
		}
		else if (bpp == BPP_32)
		{
			//std::memcpy(&rgba32, raw, width * height * (sizeof(unsigned int) / sizeof(byte)));
			rgba32 = reinterpret_cast<unsigned int*>(raw);
		}

		data.pSysMem = rgba32;

		hr = ref->sys->dx->m_d3dDevice->CreateTexture2D(&texture->m_textureDesc, &data, &texture->m_texture2D);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to create texture";
			delete[] rgba32;
			rgba32 = nullptr;
			return nullptr;
		}

		// Get texture/resource as necessary
		if ((texture->m_resource) && (!texture->m_texture2D))
		{
			hr = texture->m_resource->QueryInterface(IID_ID3D12Texture2D, (void **)&texture->m_texture2D);
			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to get Texture2D from resource.");
			}
		}
		else if ((!texture->m_resource) && (texture->m_texture2D))
		{
			hr = texture->m_texture2D->QueryInterface(IID_ID3D12Resource, (void **)&texture->m_resource);
			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to get resource from Texture2D.");
			}
		}

		// Get SRV as necessary
		if ((texture->m_resource) && (!texture->m_shaderResourceView))
		{
			hr = ref->sys->dx->m_d3dDevice->CreateShaderResourceView(texture->m_resource, NULL, &texture->m_shaderResourceView);
			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to get ShaderResourceView from resource.");
			}
		}

		// Overwrite the texture desc with whatever is in memory/on GPU
		if (texture->m_texture2D)
		{
			texture->m_texture2D->GetDesc(&texture->m_textureDesc);
		}
	}

	return texture;
}
*/

inline void dx12::ImageManager::UploadScratchImage(ScratchImage &scratch, ID3D12Resource** pResource, bool generateMipMap)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	unsigned int miscFlags = 0;
	/*
		if (generateMipMap)
		{
			miscFlags = D3D12_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = CreateTextureEx(ref->sys->dx->m_d3dDevice, scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(),
			D3D12_USAGE_DEFAULT, D3D12_BIND_SHADER_RESOURCE, 0, miscFlags, false, pResource);

		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to create resource for ScratchImage.");
		}
		*/
}

std::shared_ptr<dx12::Texture2D> dx12::ImageManager::Load(std::string name, imagetype_t type)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	byte	*buffer = nullptr;
	int bufferSize = -1;
	bool generateMipMap = ((type != it_pic) && (type != it_sky));
	unsigned int	miscFlags = 0;

	if (name.length() < 1)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, "Bad name: " + name);
	}

	// First, see if the image has already been loaded:
	std::shared_ptr<Texture2D> image = std::static_pointer_cast<Texture2D>(ref->res->GetResource(name, RESOURCE_TEXTURE2D));

	if (image)
	{
		return image;
	}
	else
	{
		// Create a new image
		auto texture = std::make_shared<Texture2D>(name);

		// Add it to the resource manager
		ref->res->AddResource(texture);

		// Determine the image type
		std::string fileName = std::filesystem::path(name).stem().string();
		std::string extension = std::filesystem::path(name).extension().string();
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
				texture->m_format = format;
				break;
			}
		}

		if (bufferSize <= 0)
		{
			ref->client->Con_Printf(PRINT_ALL, "File not found: " + name);
			return nullptr;
		}

		/*if (generateMipMap)
		{
			miscFlags = D3D12_RESOURCE_MISC_GENERATE_MIPS;
		}

		if (m_images[name]->m_format.compare("pcx") == 0)
		{
			byte	*pic = nullptr, *palette = nullptr;
			LoadPCX(buffer, bufferSize, &pic, &palette, m_images[name]->m_textureDesc.Width, m_images[name]->m_textureDesc.Height);
			m_images[name] = CreateTexture2DFromRaw(name, m_images[name]->m_textureDesc.Width, m_images[name]->m_textureDesc.Height, false, BPP_8, pic, m_8to32table, D3D12_USAGE_DEFAULT);	// FIXME
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
				0, D3D12_USAGE_DEFAULT, D3D12_BIND_SHADER_RESOURCE,
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
			hr = GetMetadataFromHDRMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), metadata);

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to extract HDR metadata from " + name + ".");
			}
			else
			{
				ScratchImage scratch;
				hr = LoadFromHDRMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), &metadata, scratch);

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
				0, D3D12_USAGE_DEFAULT, D3D12_BIND_SHADER_RESOURCE,
				NULL, miscFlags,
				WIC_LOADER_DEFAULT,
				&m_images[name]->m_resource, &m_images[name]->m_shaderResourceView);

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with WIC loader.");
			}
		}
		*/
		LOG(info) << "Successfully uploaded " << name << " to GPU.";
	}

	if (buffer)
	{
		ref->client->FS_FreeFile(buffer);
	}

	std::shared_ptr<Texture2D> texture = std::static_pointer_cast<Texture2D>(ref->res->GetResource(name, RESOURCE_TEXTURE2D));

	// Get texture/resource as necessary
	/*if ((m_images[name]->m_resource) && (!m_images[name]->m_texture2D))
	{
		hr = m_images[name]->m_resource->QueryInterface(IID_ID3D12Resource, (void **)&m_images[name]->m_texture2D);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get Texture2D from resource.");
		}
	}
	else if ((!m_images[name]->m_resource) && (m_images[name]->m_texture2D))
	{
		hr = m_images[name]->m_texture2D->QueryInterface(IID_ID3D12Resource, (void **)&m_images[name]->m_resource);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get resource from Texture2D.");
		}
	}*/

	// Get SRV as necessary
	/*if ((m_images[name]->m_resource) && (!m_images[name]->m_shaderResourceView))
	{
		hr = ref->sys->dx->m_d3dDevice->CreateShaderResourceView(m_images[name]->m_resource, NULL, &m_images[name]->m_shaderResourceView);
		if (FAILED(hr))
		{
			ref->client->Con_Printf(PRINT_ALL, "Failed to get ShaderResourceView from resource.");
		}
	}*/

	// Overwrite the texture desc with whatever is in memory/on GPU
	texture->UpdateDesc();

	texture->m_imageType = type;

	// Return the pointer
	return texture;
}

void dx12::ImageManager::SetRawPalette(const unsigned char *palette)
{
	LOG_FUNC();

	if (palette)
	{
		// Overwrite with provided palette

		/*for (i = 0; i < 256; i++)
		{
			rp[i * 4 + 0] = palette[i * 3 + 0];
			rp[i * 4 + 1] = palette[i * 3 + 1];
			rp[i * 4 + 2] = palette[i * 3 + 2];
			rp[i * 4 + 3] = 0xff;
		}*/

		//for (unsigned int i = 0; i < 256; i++)
		Concurrency::parallel_for(0u, 256u, [&m_rawPalette = m_rawPalette, &palette](unsigned int i)
		{
			// Supplied palette is BGR
			m_rawPalette[i].r = palette[i * 3 + 2];
			m_rawPalette[i].g = palette[i * 3 + 1];
			m_rawPalette[i].b = palette[i * 3 + 0];
			m_rawPalette[i].a = 255;
		});
	}
	else
	{
		// Copy back default palette

		/*for (i = 0; i < 256; i++)
		{
			rp[i * 4 + 0] = d_8to24table[i] & 0xff;
			rp[i * 4 + 1] = (d_8to24table[i] >> 8) & 0xff;
			rp[i * 4 + 2] = (d_8to24table[i] >> 16) & 0xff;
			rp[i * 4 + 3] = 0xff;
		}*/

		//for (unsigned int i = 0; i < 256; i++)
		Concurrency::parallel_for(0u, 256u, [&m_rawPalette = m_rawPalette, &m_8to32table = m_8to32table](unsigned int i)
		{
			m_rawPalette[i] = m_8to32table[i];
			m_rawPalette[i].a = 255;			// Ensure we set alpha to 1
		});
	}
}