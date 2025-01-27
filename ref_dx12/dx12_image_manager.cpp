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
	UINT64			width = 0;
	unsigned int	height = 0;

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


void dx12::ImageManager::UpdateTexture2DFromRaw(std::shared_ptr<dx12::Texture2D> texture, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR *palette)
{
	LOG_FUNC();

	D3D12_SUBRESOURCE_DATA		data;
	HRESULT hr = E_UNEXPECTED;

	try {

		if (raw != nullptr)
		{
			ZeroMemory(&texture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			ZeroMemory(&data, sizeof(D3D12_SUBRESOURCE_DATA));

			texture->m_resourceDesc.Width = width;
			texture->m_resourceDesc.Height = height;
			if (generateMipmaps)
			{
				texture->m_resourceDesc.MipLevels = 0;
			}
			else
			{
				texture->m_resourceDesc.MipLevels = 1;
			}
			texture->m_resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texture->m_resourceDesc.SampleDesc.Count = 1;
			texture->m_resourceDesc.SampleDesc.Quality = static_cast<UINT>(DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN);
			texture->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texture->m_resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texture->m_resourceDesc.DepthOrArraySize = 1;

			data.RowPitch = width * (sizeof(unsigned int) / sizeof(byte));
			data.SlicePitch = static_cast<unsigned long long>(width) * height * (sizeof(unsigned int) / sizeof(byte));
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

			data.pData = rgba32;

			//DescriptorHeap heap();

			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);


			//hr = ref->sys->dx->m_d3dDevice->CreateTexture2D(&texture->m_resourceDesc, &data, &texture->m_texture2D);

			hr = ref->sys->dx->m_d3dDevice->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&texture->m_resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(texture->m_texture.ReleaseAndGetAddressOf()));

			if (FAILED(hr))
			{
				LOG(error) << "Failed to update texture";
				delete[] rgba32;
				rgba32 = nullptr;
				return;
			}

			ResourceUploadBatch resourceUpload(ref->sys->dx->m_d3dDevice.Get());

			resourceUpload.Begin();

			resourceUpload.Upload(texture->m_texture.Get(), 0, &data, 1);

			resourceUpload.Transition(
				texture->m_texture.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			auto uploadResourcesFinished = resourceUpload.End(ref->sys->dx->m_commandQueue.Get());

			uploadResourcesFinished.wait();

			// Get SRV
			/*if ((texture->m_resource) && (!texture->m_shaderResourceView))
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = texture->m_resourceDesc.Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;

				ref->sys->dx->m_d3dDevice->CreateShaderResourceView(texture->m_resource, &srvDesc, ref->sys->dx->GetHeapCBVSRVUAV()->GetCPUDescriptorHandleForHeapStart());
				if (FAILED(hr))
				{
					ref->client->Con_Printf(PRINT_ALL, "Failed to get ShaderResourceView from resource.");
				}
			}*/

			// Overwrite the texture desc with whatever is in memory/on GPU
			if (texture)
			{
				texture->UpdateDesc();
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return;
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
	auto image = ref->res->GetResource<Texture2D>(name);

	if (image)
	{
		return image;
	}
	else
	{
		// Create a new image
		auto texture = ref->res->CreateResource<Texture2D>(name);
		texture->m_imageType = type;

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
		}*/

		if (texture->m_format.compare("pcx") == 0)
		{
			byte	*pic = nullptr, *palette = nullptr;
			LoadPCX(buffer, bufferSize, &pic, &palette, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height);
			UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, false, BPP_8, pic, m_8to32table);	// FIXME
		}
		else if (texture->m_format.compare("wal") == 0)
		{
			// Requesting a .wal file
		}
		else if (texture->m_format.compare("tga") == 0)
		{
			// Requesting a .tga file
		}
		else if (texture->m_format.compare("dds") == 0)
		{
			// .dds file

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with DDS loader.");
			}

		}
		else if (texture->m_format.compare("hdr") == 0)
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

			}
		}
		else if (texture->m_format.compare("exr") == 0)
		{
			// Requesting a .exr file
		}
		else
		{
			// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, "Failed to load " + name + " with WIC loader.");
			}
		}
		
		LOG(info) << "Successfully uploaded " << texture->GetName() << " to GPU.";
	}

	if (buffer)
	{
		ref->client->FS_FreeFile(buffer);
	}

	auto texture = ref->res->GetResource<Texture2D>(name);

	if (texture)
	{
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
	}

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