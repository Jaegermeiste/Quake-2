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

	m_whiteTexture = Load(WHITE_TEXTURE_NAME, it_pic);
	m_checkerboardTexture = Load(CHECKERBOARD_TEXTURE_NAME, it_pic);
	m_conChars = Load(L"pics/conchars.pcx", it_pic);

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

	if (m_conChars)
	{
		SAFE_RELEASE(m_conChars->m_resource);
		ZeroMemory(&m_conChars->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
		m_conChars->m_format.clear();
		m_conChars->m_registrationSequence = 0;
		m_conChars = nullptr;
	}

	if (m_whiteTexture)
	{
		SAFE_RELEASE(m_whiteTexture->m_resource);
		ZeroMemory(&m_whiteTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
		m_whiteTexture->m_format.clear();
		m_whiteTexture->m_registrationSequence = 0;
		m_whiteTexture = nullptr;
	}

	if (m_checkerboardTexture)
	{
		SAFE_RELEASE(m_checkerboardTexture->m_resource);
		ZeroMemory(&m_checkerboardTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
		m_checkerboardTexture->m_format.clear();
		m_checkerboardTexture->m_registrationSequence = 0;
		m_checkerboardTexture = nullptr;
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
	int len = ref->client->FS_LoadFile(L"pics/colormap.pcx", (void **)&raw);
	if (!raw)
	{
		ref->client->Sys_Error(ERR_FATAL, L"Couldn't load pics/colormap.pcx");
		return;
	}

	// get the palette
	LOG(info) << "Loading pics/colormap.pcx";
	LoadPCX(raw, len, &pic, &pal, width, height);

	if (!pal)
	{
		ref->client->Sys_Error(ERR_FATAL, L"Couldn't load pics/colormap.pcx");
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

std::shared_ptr<dx12::Texture2D> dx12::ImageManager::CreateTexture2DFromRaw(std::wstring name, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	byte* buffer = nullptr;
	int bufferSize = -1;
	unsigned int	miscFlags = 0;

	if (name.length() < 1)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, L"Bad name: " + name);
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

		if (texture)
		{
			texture->m_imageType = it_pic;
			texture->m_format = L"RAW";

			texture->m_resourceDesc.Width = width;
			texture->m_resourceDesc.Height = height;
			texture->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texture->m_resourceDesc.DepthOrArraySize = 1;
			if (generateMipmaps)
			{
				texture->m_resourceDesc.MipLevels = 0;
			}
			else
			{
				texture->m_resourceDesc.MipLevels = 1;
			}
			texture->m_resourceDesc.SampleDesc.Count = 1;
			texture->m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			if (bpp == BPP_8)
			{
				UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, generateMipmaps, BPP_8, raw, m_8to32table, false);
			}
			else
			{
				UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, generateMipmaps, bpp, raw, nullptr, false);
			}

			if (texture->m_resource)
			{
				texture->m_resource->SetName(name.c_str());
			}

			LOG(info) << "Successfully uploaded " << texture->GetName() << " to GPU.";
		}
	}

	auto texture = ref->res->GetResource<Texture2D>(name);

	if (texture)
	{
		// Overwrite the texture desc with whatever is in memory/on GPU
		texture->UpdateDesc();

		// Create a shader resource view
		texture->CreateSRV();
	}

	// Return the pointer
	return texture;
}

void dx12::ImageManager::UpdateTexture2DFromRaw(std::shared_ptr<dx12::Texture2D> texture, unsigned int width, unsigned int height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR *palette, bool update, std::shared_ptr<CommandList> commandList)
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

			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

			hr = ref->sys->dx->m_d3dDevice->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&texture->m_resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(texture->m_resource.ReleaseAndGetAddressOf()));

			if (FAILED(hr))
			{
				LOG(error) << "Failed to update texture: " << GetD3D12ErrorMessage(hr);
				delete[] rgba32;
				rgba32 = nullptr;
				return;
			}

			ResourceUploadBatch resourceUpload(ref->sys->dx->m_d3dDevice.Get());

			resourceUpload.Begin();

			if (update && commandList && commandList->IsOpen())
			{
				auto uploadHeap = ref->res->GetOrCreateResource<Resource>(L"UploadHeap_" + texture->GetName());

				// Define heap properties for an upload buffer
				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;  // Must be UPLOAD type for CPU -> GPU transfers

				// Describe the upload buffer
				uploadHeap->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				uploadHeap->m_resourceDesc.Width = sizeof(unsigned int) * static_cast<UINT64>(width) * height;
				uploadHeap->m_resourceDesc.Height = 1;
				uploadHeap->m_resourceDesc.DepthOrArraySize = 1;
				uploadHeap->m_resourceDesc.MipLevels = 1;
				uploadHeap->m_resourceDesc.SampleDesc.Count = 1;
				uploadHeap->m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				// Create the upload heap resource
				hr = ref->sys->dx->Device()->CreateCommittedResource(
					&heapProps,               // Heap properties
					D3D12_HEAP_FLAG_NONE,     // Heap flag
					&uploadHeap->m_resourceDesc,              // Resource description
					D3D12_RESOURCE_STATE_GENERIC_READ,  // Must be GENERIC_READ for upload heap
					nullptr,                  // No clear value needed
					IID_PPV_ARGS(&uploadHeap->m_resource)  // Output resource
				);

				if (FAILED(hr)) {
					LOG(error) << "Failed to create upload heap.\n";
				}


				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = texture->m_resource.Get();
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				commandList->List()->ResourceBarrier(1, &barrier);

				// Update the texture resource
                UpdateSubresources(
					commandList->List().Get(),
                    texture->m_resource.Get(),
					uploadHeap->m_resource.Get(),
                    0,
                    0,
                    1,
                    &data
                );

				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				commandList->List()->ResourceBarrier(1, &barrier);

				texture->m_srvHandle = 0;
				texture->m_cachedD3D12SRVHandle = {};			
			}
			else
			{
				resourceUpload.Upload(texture->m_resource.Get(), 0, &data, 1);
			}

			resourceUpload.Transition(
				texture->m_resource.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			if (generateMipmaps) {
				resourceUpload.GenerateMips(texture->m_resource.Get());
			}

			auto uploadResourcesFinished = resourceUpload.End(ref->sys->dx->m_commandQueue.Get());

			uploadResourcesFinished.wait();

			// Overwrite the texture desc with whatever is in memory/on GPU
			if (texture)
			{
				texture->UpdateDesc();

				texture->RefreshSRV();
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


std::shared_ptr<dx12::Texture2D> dx12::ImageManager::Load(std::wstring name, imagetype_t type)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;
	byte	*buffer = nullptr;
	int bufferSize = -1;
	bool generateMipMap = ((type != it_pic) && (type != it_sky));
	unsigned int	miscFlags = 0;

	if (name.length() < 1)
	{
		ref->client->Con_Printf(PRINT_DEVELOPER, L"Bad name: " + name);
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

		if ((name.compare(WHITE_TEXTURE_NAME) != 0) && (name.compare(CHECKERBOARD_TEXTURE_NAME) != 0))
		{
			// Determine the image type
			std::wstring fileName = std::filesystem::path(name).stem().wstring();
			std::wstring extension = std::filesystem::path(name).extension().wstring();
			std::wstring path = L"";

			if ((fileName.at(0) != '/') && (fileName.at(0) != '\\'))
			{
				switch (type)
				{
				case it_pic:
					path = L"pics/";
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
			for (auto& format : m_imageExtensions)
			{
				buffer = nullptr;
				bufferSize = -1;

				// Attempt to load the file
				bufferSize = ref->client->FS_LoadFile(path + fileName + L"." + format, (void**)&buffer);

				if ((bufferSize > 0) && (buffer))
				{
					texture->m_format = format;
					break;
				}
			}

			if (bufferSize <= 0)
			{
				ref->client->Con_Printf(PRINT_ALL, L"File not found: " + name);
				return nullptr;
			}
		}
		else 
		{
			texture->m_format = L"INTERNAL";
		}

		/*if (generateMipMap)
		{
			miscFlags = D3D12_RESOURCE_MISC_GENERATE_MIPS;
		}*/

		if (texture->m_format.compare(L"INTERNAL") == 0)
		{
			if (name.compare(WHITE_TEXTURE_NAME) == 0)
			{
				UINT8 whitePixel[4] = { 255, 255, 255, 255 };
				texture->m_resourceDesc.Width = 1;
				texture->m_resourceDesc.Height = 1;
				texture->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texture->m_resourceDesc.DepthOrArraySize = 1;
				texture->m_resourceDesc.MipLevels = 1;
				texture->m_resourceDesc.SampleDesc.Count = 1;
				texture->m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, false, BPP_32, whitePixel, nullptr, false);
			}
			else if (name.compare(CHECKERBOARD_TEXTURE_NAME) == 0)
			{
				const int dimension = 256;
				std::vector<UINT8> checkerboardData(dimension * dimension * 4); // RGBA
				int squareSize = dimension / 8; // 8x8 checkerboard pattern

				for (int y = 0; y < dimension; y++) {
					for (int x = 0; x < dimension; x++) {
						int index = (y * dimension + x) * 4;

						// Checkerboard pattern logic
						bool isWhite = ((x / squareSize) % 2) ^ ((y / squareSize) % 2);
						UINT8 color = isWhite ? 255 : 0; // White (255,255,255), Black (0,0,0)

						checkerboardData[index]     = color; // R
						checkerboardData[index + 1] = color; // G
						checkerboardData[index + 2] = color; // B
						checkerboardData[index + 3] = 255;   // A (Opaque)
					}
				}

				texture->m_resourceDesc.Width = dimension;
				texture->m_resourceDesc.Height = dimension;
				texture->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texture->m_resourceDesc.DepthOrArraySize = 1;
				texture->m_resourceDesc.MipLevels = 0;
				texture->m_resourceDesc.SampleDesc.Count = 1;
				texture->m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, true, BPP_32, checkerboardData.data(), nullptr, false);
			}
		}
		else if (texture->m_format.compare(L"pcx") == 0)
		{
			byte	*pic = nullptr, *palette = nullptr;
			LoadPCX(buffer, bufferSize, &pic, &palette, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height);
			UpdateTexture2DFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, false, BPP_8, pic, m_8to32table, false);	// FIXME
		}
		else if (texture->m_format.compare(L"wal") == 0)
		{
			// Requesting a .wal file
		}
		else if (texture->m_format.compare(L"tga") == 0)
		{
			// Requesting a .tga file
		}
		else if (texture->m_format.compare(L"dds") == 0)
		{
			// .dds file

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with DDS loader.");
			}

		}
		else if (texture->m_format.compare(L"hdr") == 0)
		{
			// Requesting a .hdr file
			TexMetadata metadata;
            if (buffer != nullptr)
            {
                hr = GetMetadataFromHDRMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), metadata);

				if (FAILED(hr))
				{
					ref->client->Con_Printf(PRINT_ALL, L"Failed to extract HDR metadata from " + name + L".");
				}
            }
            else
            {
                hr = E_POINTER; // or any appropriate error code
                ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
            }
		}
		else if (texture->m_format.compare(L"exr") == 0)
		{
			// Requesting a .exr file
		}
		else
		{
			// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)

			if (FAILED(hr))
			{
				ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with WIC loader.");
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
		// Overwrite the texture desc with whatever is in memory/on GPU
		texture->UpdateDesc();

		// Create a shader resource view
		texture->CreateSRV();
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