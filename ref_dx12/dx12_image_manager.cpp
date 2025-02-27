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

extern "C" __declspec(dllexport) void SHIM_D3D_Imagelist_f(void)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->img != nullptr))
		{
			dx12::ref->media->img->Imagelist_f();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

bool dx12::ImageManager::Initialize()
{
	LOG_FUNC();

	try
	{
		m_resourceUploadBatch = std::make_unique<ResourceUploadBatch>(ref->sys->dx->Device().Get());

		GetPalette();

		m_whiteTexture = Load(WHITE_TEXTURE_NAME, it_pic);
		m_checkerboardTexture = Load(CHECKERBOARD_TEXTURE_NAME, it_pic);
		m_conChars = Load(L"pics/conchars.pcx", it_pic);

#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
		Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
		if (FAILED(initialize))
		{
			LOG(error) << "Failed to initialize WRL::WIC!";
		}
#else
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			LOG(error) << "Failed to initialize WRL::WIC!";
		}
#endif	
		// Create command
		ref->client->Cmd_AddCommand(L"dx12_imagelist", SHIM_D3D_Imagelist_f);

		return true;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

void dx12::ImageManager::Shutdown()
{
	LOG_FUNC();

	try
	{
		LOG(info) << "Shutting down.";

		if (m_rawTexture)
		{
			SAFE_RELEASE(m_rawTexture->m_resource);
			ZeroMemory(&m_rawTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			m_rawTexture->m_fileFormat.clear();
			m_rawTexture->m_registrationSequence = 0;
			m_rawTexture = nullptr;
		}

		if (m_conChars)
		{
			SAFE_RELEASE(m_conChars->m_resource);
			ZeroMemory(&m_conChars->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			m_conChars->m_fileFormat.clear();
			m_conChars->m_registrationSequence = 0;
			m_conChars = nullptr;
		}

		if (m_whiteTexture)
		{
			SAFE_RELEASE(m_whiteTexture->m_resource);
			ZeroMemory(&m_whiteTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			m_whiteTexture->m_fileFormat.clear();
			m_whiteTexture->m_registrationSequence = 0;
			m_whiteTexture = nullptr;
		}

		if (m_checkerboardTexture)
		{
			SAFE_RELEASE(m_checkerboardTexture->m_resource);
			ZeroMemory(&m_checkerboardTexture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC));
			m_checkerboardTexture->m_fileFormat.clear();
			m_checkerboardTexture->m_registrationSequence = 0;
			m_checkerboardTexture = nullptr;
		}

		m_textures.clear();

		if (m_resourceUploadBatch)
		{
			SAFE_RELEASE(m_resourceUploadBatch);
		}

		LOG(info) << "Shutdown complete.";
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

/*
===============
GetPalette
===============
*/
void dx12::ImageManager::GetPalette(void)
{
	LOG_FUNC();

	try
	{
		byte* raw = nullptr,
			* pic = nullptr,
			* pal = nullptr;
		size_t			width = 0, height = 0;

		//
		// load the file
		//
		int len = ref->client->FS_LoadFile(L"pics/colormap.pcx", (void**)&raw);
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
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

std::shared_ptr<dx12::Texture> dx12::ImageManager::CreateTextureFromRaw(std::wstring name, size_t width, size_t height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette)
{
	LOG_FUNC();

	try
	{
		HRESULT hr = E_UNEXPECTED;
		byte* buffer = nullptr;
		size_t bufferSize = 0;
		unsigned int	miscFlags = 0;

		if (name.length() < 1)
		{
			ref->client->Con_Printf(PRINT_DEVELOPER, L"Bad name: " + name);
		}

		// First, see if the image has already been loaded:
		auto image = ref->res->GetResource<Texture>(name);

		if (image)
		{
			return image;
		}
		else
		{
			// Create a new image
			auto texture = CreateTexture(name, it_pic, width, height, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1);

			if (texture)
			{
				UpdateTextureFromRaw(texture, width, height, generateMipmaps, bpp, raw, palette);

				if (texture->m_resource)
				{
					texture->m_resource->SetName(name.c_str());
				}

				return texture;
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

void dx12::ImageManager::UpdateTextureFromRaw(std::shared_ptr<Texture> texture, size_t width, size_t height, bool generateMipmaps, unsigned int bpp, byte* raw, XMCOLOR* palette)
{
	LOG_FUNC();

	try
	{
		HRESULT hr = E_UNEXPECTED;
		byte* buffer = nullptr;
		int bufferSize = -1;
		unsigned int	miscFlags = 0;

		if (!texture)
		{
			ref->client->Con_Printf(PRINT_DEVELOPER, L"Bad texture! ");
		}

		if (texture)
		{
			texture->m_imageType = it_pic;
			texture->m_fileFormat = L"RAW";

			texture->m_resourceDesc.Width = msl::utilities::SafeInt<UINT64>(width);
			texture->m_resourceDesc.Height = msl::utilities::SafeInt<UINT>(height);
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

			texture->m_subresources.clear();
			texture->m_subresources.shrink_to_fit();
			texture->m_subresources.resize(1);
			if (bpp == BPP_8)
			{
				texture->m_subresources[0] = DepalletizeRawData(width, height, BPP_8, raw, palette);
			}
			else
			{
				texture->m_subresources[0] = DepalletizeRawData(width, height, bpp, raw, nullptr);
			}

			UpdateTexture(texture);

			LOG(info) << "Successfully uploaded " << texture->GetName() << " to GPU.";
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}


void dx12::ImageManager::Imagelist_f()
{
	LOG_FUNC();

	try
	{
		ref->client->Con_Printf(PRINT_ALL, L"-----------------------------------------------------------------------------------------------------------");
		ref->client->Con_Printf(PRINT_ALL, L"\t SEQ |   TYPE    |   W  x  H   |                 NAME                 | DXGI FORMAT");
		ref->client->Con_Printf(PRINT_ALL, L"-----------------------------------------------------------------------------------------------------------");
		for (auto texture : m_textures)
		{
			if (texture)
			{
				ref->client->Con_Printf(PRINT_ALL, std::format(L"\t{:4} | {:9} | {:4} x {:4} | {:36} | {}",
					texture->m_registrationSequence,
					ref->sys->ToWideString(magic_enum::enum_name(texture->m_imageType)),
					texture->GetWidth(),
					texture->GetHeight(),
					texture->GetName(),
					ref->sys->ToWideString(magic_enum::enum_name(texture->GetDXGIFormat()))
					));
			}
		}
		ref->client->Con_Printf(PRINT_ALL, L"-----------------------------------------------------------------------------------------------------------");
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

std::shared_ptr<dx12::Texture> dx12::ImageManager::CreateTexture(std::wstring name, imagetype_t type, size_t width, size_t height, bool generateMipmaps, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, size_t arraySize = 1)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try {
		auto texture = ref->res->CreateResource<Texture>(name);

		texture->m_imageType = type;

		ZeroMemory(&texture->m_resourceDesc, sizeof(D3D12_RESOURCE_DESC1));

		texture->m_resourceDesc.Width = msl::utilities::SafeInt<UINT64>(width);
		texture->m_resourceDesc.Height = msl::utilities::SafeInt<UINT>(height);
		if (generateMipmaps)
		{
			texture->m_resourceDesc.MipLevels = 0;
		}
		else
		{
			texture->m_resourceDesc.MipLevels = 1;
		}
		texture->m_resourceDesc.Format = format;
		texture->m_resourceDesc.SampleDesc.Count = 1;
		texture->m_resourceDesc.SampleDesc.Quality = static_cast<UINT>(DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN);
		texture->m_resourceDesc.Dimension = dimension;
		texture->m_resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texture->m_resourceDesc.DepthOrArraySize = msl::utilities::SafeInt<UINT16>(arraySize);

		ClearSubresources(texture, arraySize);

		if (arraySize == 6)
		{
			texture->m_cubemap = true;
		}
		else
		{
			texture->m_cubemap = false;
		}

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

		hr = ref->sys->dx->Device()->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texture->m_resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			nullptr,
			IID_PPV_ARGS(texture->m_resource.ReleaseAndGetAddressOf()));

		if (FAILED(hr))
		{
			LOG(error) << "Failed to create texture: " << GetD3D12ErrorMessage(hr);
			return nullptr;
		}

		texture->m_resource->SetName(name.c_str());

		m_textures.push_back(texture);

		return texture;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

std::shared_ptr<dx12::Texture> dx12::ImageManager::CreateTexture(std::wstring name, imagetype_t type, size_t width, size_t height, bool generateMipmaps, DXGI_FORMAT format, TEX_DIMENSION dimension, size_t arraySize)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try {
		D3D12_RESOURCE_DIMENSION d3d12Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;

		switch (dimension)
		{
		case TEX_DIMENSION_TEXTURE1D:
			d3d12Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case TEX_DIMENSION_TEXTURE3D:
			d3d12Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		case TEX_DIMENSION_TEXTURE2D:
		default:
			d3d12Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		}

		return CreateTexture(name, type, width, height, generateMipmaps, format, d3d12Dimension, arraySize);
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

void dx12::ImageManager::UpdateTexture(std::shared_ptr<dx12::Texture> texture)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try {
		if (!texture->m_subresources.empty())
		{
			UploadSubresources(texture);
		}

		// Overwrite the texture desc with whatever is in memory/on GPU
		if (texture)
		{
			texture->UpdateDesc();

			texture->RefreshSRV();
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

std::vector<D3D12_SUBRESOURCE_DATA> dx12::ImageManager::GetSubresourcesFromScratch(DirectX::ScratchImage& scratch)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	std::vector<D3D12_SUBRESOURCE_DATA> data = {};

	try {
		const DirectX::Image* images = scratch.GetImages();
		size_t imageCount = scratch.GetImageCount();
		data.resize(imageCount);

		for (size_t i = 0; i < imageCount; ++i) {
			ZeroMemory(&data[i], sizeof(D3D12_SUBRESOURCE_DATA));
			data[i].pData = images[i].pixels;
			data[i].RowPitch = images[i].rowPitch;
			data[i].SlicePitch = images[i].slicePitch;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return data;
}

std::vector<D3D12_SUBRESOURCE_DATA> dx12::ImageManager::GetSubresourcesFromResource(ComPtr<ID3D12Resource> resource)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	std::vector<D3D12_SUBRESOURCE_DATA> subresourceData = {};
	try {
		if (!resource) {
			return subresourceData;
		}

		// Map the resource to access its data
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read from this resource on the CPU
		uint8_t* mappedData = nullptr;
		hr = resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));

		if (FAILED(hr)) {
			LOG(error) << "Failed to map resource: " << GetD3D12ErrorMessage(hr);
			return subresourceData;
		}

		// Get resource description
		D3D12_RESOURCE_DESC desc = resource->GetDesc();

		// Compute subresource size and pitch
		for (UINT subresourceIndex = 0; subresourceIndex < desc.DepthOrArraySize; ++subresourceIndex) {
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
			UINT numRows = 0;
			size_t rowSizeInBytes = 0;
			size_t totalBytes = 0;

			ComPtr<ID3D12Device> device;
			resource->GetDevice(IID_PPV_ARGS(&device));
			device->GetCopyableFootprints(&desc, subresourceIndex, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

			D3D12_SUBRESOURCE_DATA data = {};
			data.pData = mappedData + footprint.Offset;
			data.RowPitch = footprint.Footprint.RowPitch;
			data.SlicePitch = footprint.Footprint.RowPitch * static_cast<size_t>(numRows);
			subresourceData.push_back(data);
		}

		// Unmap the resource
		resource->Unmap(0, nullptr);
		return subresourceData;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return subresourceData;
}

std::shared_ptr<dx12::Texture> dx12::ImageManager::Load(std::wstring name, imagetype_t type)
{
	LOG_FUNC();

	try
	{
		HRESULT hr = E_UNEXPECTED;
		byte* buffer = nullptr;
		size_t bufferSize = 0;
		bool generateMipMap = ((type != it_pic) && (type != it_sky));
		unsigned int	miscFlags = 0;

		if (name.length() < 1)
		{
			ref->client->Con_Printf(PRINT_DEVELOPER, L"Bad name: " + name);
		}

		// First, see if the image has already been loaded:
		auto image = ref->res->GetResource<Texture>(name);

		if (image)
		{
			image->m_registrationSequence = ref->media->RegistrationSequence();
			return image;
		}
		else
		{
			// Create a new image
			std::shared_ptr<Texture> texture = nullptr;

			std::wstring fileFormat = L"";

			if ((name.compare(WHITE_TEXTURE_NAME) != 0) && (name.compare(CHECKERBOARD_TEXTURE_NAME) != 0))
			{
				buffer = LoadImageFile(name, type, fileFormat, bufferSize);

				if ((bufferSize <= 0) || (!buffer))
				{
					LOG(warning) << "Failed to load image: " << name;
				}
			}
			else
			{
				fileFormat = L"INTERNAL";
			}

			if (fileFormat.compare(L"INTERNAL") == 0)
			{

				if (name.compare(WHITE_TEXTURE_NAME) == 0)
				{
					UINT8 whitePixel[4] = { 255, 255, 255, 255 };

					texture = CreateTexture(name, it_pic, 1, 1, false);

					texture->m_subresources[0] = DepalletizeRawData(1, 1, BPP_32, whitePixel, nullptr);
				}
				else if (name.compare(CHECKERBOARD_TEXTURE_NAME) == 0)
				{
					constexpr size_t checkerboardSizePixels = 256;
					auto bufferSize = checkerboardSizePixels * checkerboardSizePixels * (BPP_32 / (sizeof(byte) * 8));
					auto checkerboardData = new byte[bufferSize]; // RGBA
					size_t squareSize = checkerboardSizePixels / 8; // 8x8 checkerboard pattern

					for (size_t y = 0; y < checkerboardSizePixels; y++) {
						for (size_t x = 0; x < checkerboardSizePixels; x++) {
							size_t index = (y * checkerboardSizePixels + x) * 4;

							// Checkerboard pattern logic
							bool isWhite = ((((x / squareSize) % 2) ^ ((y / squareSize) % 2)) == 0);
							UINT8 color = isWhite ? 255 : 0; // White (255,255,255), Black (0,0,0)

							checkerboardData[index] = color; // R
							checkerboardData[index + 1] = color; // G
							checkerboardData[index + 2] = color; // B
							checkerboardData[index + 3] = 255;   // A (Opaque)
						}
					}

					texture = CreateTexture(name, it_pic, checkerboardSizePixels, checkerboardSizePixels, true);

					texture->m_subresources[0] = DepalletizeRawData(checkerboardSizePixels, checkerboardSizePixels, BPP_32, checkerboardData, nullptr);

					//delete[] checkerboardData;
					//checkerboardData = nullptr;
				}
			}
			else if (fileFormat.compare(L"pcx") == 0)
			{
				byte* pic = nullptr, * palette = nullptr;
				size_t width = 0, height = 0;
				LoadPCX(buffer, bufferSize, &pic, &palette, width, height);

				if (pic)
				{
					texture = CreateTexture(name, type, width, height, generateMipMap);

					/*if (palette)
					{
						texture->m_subresources[0] = DepalletizeRawData(width, height, BPP_8, pic, palette);
					}
					else*/
					{
						texture->m_subresources[0] = DepalletizeRawData(width, height, BPP_8, pic, m_8to32table);
					}
				}
			}
			else if (fileFormat.compare(L"wal") == 0)
			{
				// Requesting a .wal file
				byte* pic = nullptr;
				size_t width = 0, height = 0;
				LoadWAL(buffer, bufferSize, &pic, width, height);

				if (pic)
				{
					texture = CreateTexture(name, type, width, height, generateMipMap);

					texture->m_subresources[0] = DepalletizeRawData(width, height, BPP_8, pic, m_8to32table);
				}
			}
			else if (fileFormat.compare(L"tga") == 0)
			{
				// Requesting a .tga file
				/*byte* pic = nullptr;
				LoadTGA(buffer, bufferSize, &pic, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height);
				UpdateTextureFromRaw(texture, texture->m_resourceDesc.Width, texture->m_resourceDesc.Height, generateMipMap, BPP_32, pic, nullptr, false);*/

				if (buffer != nullptr)
				{
					ScratchImage scratch = {};
					hr = LoadFromTGAMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), nullptr, scratch);

					if (FAILED(hr))
					{
						ref->client->Con_Printf(PRINT_ALL, L"Failed to load TGA " + name + L".");
					}
					else
					{
						texture = CreateTexture(name, type, scratch.GetMetadata().width, scratch.GetMetadata().height, scratch.GetMetadata().mipLevels <= 1, scratch.GetMetadata().format, scratch.GetMetadata().dimension, scratch.GetMetadata().arraySize);
						texture->m_resourceDesc.MipLevels = msl::utilities::SafeInt<UINT16>(scratch.GetMetadata().mipLevels);

						texture->m_subresources = GetSubresourcesFromScratch(scratch);
					}
				}
				else
				{
					hr = E_POINTER; // or any appropriate error code
					ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
				}
			}
			else if (fileFormat.compare(L"dds") == 0)
			{
				// .dds file
				if (!buffer || bufferSize < sizeof(dds_header_t)) {
					return nullptr;
				}

				const dds_header_t* header = reinterpret_cast<const dds_header_t*>(buffer);

				if (header->size != sizeof(dds_header_t)) {
					return nullptr;
				}

				texture = CreateTexture(name, type, header->width, header->height, header->mipMapCount <= 1);
				texture->m_resourceDesc.MipLevels = msl::utilities::SafeInt<UINT16>(header->mipMapCount);

				ClearSubresources(texture, 0);

				m_resourceUploadBatch->Begin();

				ComPtr<ID3D12Resource> resource = nullptr;

				hr = texture->m_resource.As(&resource);

				if (FAILED(hr))
				{
					LOG(error) << "Failed to obtain ID3D12Resource from ID3D12Resource2!";
				}

				hr = CreateDDSTextureFromMemory(
					ref->sys->dx->Device().Get(),
					*m_resourceUploadBatch,
					buffer,
					bufferSize,
					resource.ReleaseAndGetAddressOf(),
					generateMipMap,
					0,
					nullptr,
					&texture->m_cubemap);

				if (FAILED(hr))
				{
					ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with DDS loader.");
				}

				// Upload the resources to the GPU.
				auto finish = m_resourceUploadBatch->End(ref->sys->dx->m_commandQueue.Get());

				// Wait for the upload thread to terminate
				finish.wait();
			}
			else if (fileFormat.compare(L"hdr") == 0)
			{
				// Requesting a .hdr file
				if (buffer != nullptr)
				{
					ScratchImage scratch = {};
					hr = LoadFromHDRMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), nullptr, scratch);

					if (FAILED(hr))
					{
						ref->client->Con_Printf(PRINT_ALL, L"Failed to load HDR " + name + L".");
					}
					else
					{
						texture = CreateTexture(name, type, scratch.GetMetadata().width, scratch.GetMetadata().height, scratch.GetMetadata().mipLevels <= 1, scratch.GetMetadata().format, scratch.GetMetadata().dimension, scratch.GetMetadata().arraySize);
						texture->m_resourceDesc.MipLevels = msl::utilities::SafeInt<UINT16>(scratch.GetMetadata().mipLevels);

						texture->m_subresources = GetSubresourcesFromScratch(scratch);
					}
				}
				else
				{
					hr = E_POINTER; // or any appropriate error code
					ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
				}
			}
			else if (fileFormat.compare(L"exr") == 0)
			{
				// Requesting a .exr file
				if (buffer != nullptr)
				{
					ScratchImage scratch = {};
					hr = LoadFromEXRFile(name.c_str(), nullptr, scratch);

					if (FAILED(hr))
					{
						ref->client->Con_Printf(PRINT_ALL, L"Failed to load EXR: " + name + L".");
					}
					else
					{
						texture = CreateTexture(name, type, scratch.GetMetadata().width, scratch.GetMetadata().height, scratch.GetMetadata().mipLevels <= 1, scratch.GetMetadata().format, scratch.GetMetadata().dimension, scratch.GetMetadata().arraySize);
						texture->m_resourceDesc.MipLevels = msl::utilities::SafeInt<UINT16>(scratch.GetMetadata().mipLevels);

						texture->m_subresources = GetSubresourcesFromScratch(scratch);
					}
				}
				else
				{
					hr = E_POINTER; // or any appropriate error code
					ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
				}
			}
			else
			{
				// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)

				texture = CreateTexture(name, type, 1, 1, generateMipMap);

				ClearSubresources(texture, 0);

				m_resourceUploadBatch->Begin();

				ComPtr<ID3D12Resource> resource = nullptr;

				hr = texture->m_resource.As(&resource);

				if (FAILED(hr))
				{
					LOG(error) << "Failed to obtain ID3D12Resource from ID3D12Resource2!";
				}

				hr = CreateWICTextureFromMemory(
					ref->sys->dx->Device().Get(),
					*m_resourceUploadBatch,
					buffer,
					bufferSize,
					resource.ReleaseAndGetAddressOf(),
					generateMipMap,
					0);

				if (FAILED(hr))
				{
					ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with WIC loader.");
				}

				// Upload the resources to the GPU.
				auto finish = m_resourceUploadBatch->End(ref->sys->dx->m_commandQueue.Get());

				// Wait for the upload thread to terminate
				finish.wait();
			}
		}

		if (buffer)
		{
			ref->client->FS_FreeFile(buffer);
		}

		auto texture = ref->res->GetResource<Texture>(name);

		if (texture)
		{
			UpdateTexture(texture);

			texture->m_registrationSequence = ref->media->RegistrationSequence();

			
		}

		// Return the pointer
		return texture;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

std::shared_ptr<dx12::Texture> dx12::ImageManager::LoadSky(std::wstring name)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	//const std::wstring suffixes[6] = { L"rt", L"bk", L"lf", L"ft", L"up", L"dn" }; // 3dstudio environment map names
	const std::wstring suffixes[6] = { L"rt", L"lf", L"up", L"dn", L"ft", L"bk" }; // 3dstudio environment map names, ordered for cubemap
	UINT64 totalSliceSize = 0;

	try {
		auto cubemap = ref->res->GetResource<Texture>(name + L"_sky");

		if (cubemap)
		{
			cubemap->m_registrationSequence = ref->media->RegistrationSequence();
			return cubemap;
		}
		else
		{
			// Try and load a cubemap
			auto image = Load(name, it_sky);

			if (image && image->IsCubemap())
			{
				image->m_registrationSequence = ref->media->RegistrationSequence();
				return image;
			}

			std::vector<D3D12_SUBRESOURCE_DATA> subresources = {};
			subresources.resize(6);
			size_t width = 0, height = 0;
			bool validCubeData = true;
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

			// Load the individual textures
			for (int i = 0; i < 6; i++)
			{
				if (validCubeData)
				{
					byte* buffer = {};
					size_t bufferSize = 0;
					std::wstring fileFormat = L"";

					buffer = LoadImageFile(L"env/" + name + suffixes[i] + L".pcx", it_sky, fileFormat, bufferSize);

					if (buffer && (fileFormat.compare(L"") != 0))
					{
						size_t currentWidth = 0, currentHeight = 0;

						if (fileFormat.compare(L"pcx") == 0)
						{
							byte* pic = nullptr, * palette = nullptr;
							LoadPCX(buffer, bufferSize, &pic, &palette, currentWidth, currentHeight);
							subresources[i] = DepalletizeRawData(currentWidth, currentHeight, BPP_8, pic, m_8to32table);
							format = DXGI_FORMAT_R8G8B8A8_UNORM;
						}
						else if (fileFormat.compare(L"wal") == 0)
						{
							byte* pic = nullptr;
							LoadWAL(buffer, bufferSize, &pic, currentWidth, currentHeight);
							subresources[i] = DepalletizeRawData(currentWidth, currentHeight, BPP_8, pic, m_8to32table);
							format = DXGI_FORMAT_R8G8B8A8_UNORM;
						}
						else if (fileFormat.compare(L"tga") == 0)
						{
							// Requesting a .tga file
							if (buffer != nullptr)
							{
								ScratchImage scratch = {};
								hr = LoadFromTGAMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), nullptr, scratch);

								if (FAILED(hr))
								{
									ref->client->Con_Printf(PRINT_ALL, L"Failed to load TGA " + name + L".");
								}
								else
								{
									currentWidth = scratch.GetMetadata().width;
									currentHeight = scratch.GetMetadata().height;
									format = scratch.GetMetadata().format;

									auto result = GetSubresourcesFromScratch(scratch);

									if (result.size() >= 1)
									{
										subresources[i] = result[0];
									}
								}
							}
							else
							{
								hr = E_POINTER; // or any appropriate error code
								ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
							}
						}
						else if (fileFormat.compare(L"dds") == 0)
						{
							// .dds file
							if (!buffer || bufferSize < sizeof(dds_header_t)) {
								return nullptr;
							}

							const dds_header_t* header = reinterpret_cast<const dds_header_t*>(buffer);

							if (header->size != sizeof(dds_header_t)) {
								return nullptr;
							}

							ComPtr<ID3D12Resource> tempResource = nullptr;
							bool isCubemap = false;

							m_resourceUploadBatch->Begin();

							hr = CreateDDSTextureFromMemory(
								ref->sys->dx->Device().Get(),
								*m_resourceUploadBatch,
								buffer,
								bufferSize,
								tempResource.ReleaseAndGetAddressOf(),
								false,
								0,
								nullptr,
								&isCubemap);

							if (FAILED(hr))
							{
								ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with DDS loader.");
							}

							// Upload the resources to the GPU.
							auto finish = m_resourceUploadBatch->End(ref->sys->dx->m_commandQueue.Get());

							// Wait for the upload thread to terminate
							finish.wait();

							if (tempResource)
							{
								currentWidth = tempResource->GetDesc().Width;
								currentHeight = tempResource->GetDesc().Height;
								format = tempResource->GetDesc().Format;

								auto result = GetSubresourcesFromResource(tempResource);

								if (result.size() >= 1)
								{
									subresources[i] = result[0];
								}

								SAFE_RELEASE(tempResource);
							}
						}
						else if (fileFormat.compare(L"hdr") == 0)
						{
							// Requesting a .hdr file
							if (buffer != nullptr)
							{
								ScratchImage scratch = {};
								hr = LoadFromHDRMemory(buffer, msl::utilities::SafeInt<size_t>(bufferSize), nullptr, scratch);

								if (FAILED(hr))
								{
									ref->client->Con_Printf(PRINT_ALL, L"Failed to load HDR " + name + L".");
								}
								else
								{
									currentWidth = scratch.GetMetadata().width;
									currentHeight = scratch.GetMetadata().height;
									format = scratch.GetMetadata().format;

									auto result = GetSubresourcesFromScratch(scratch);

									if (result.size() >= 1)
									{
										subresources[i] = result[0];
									}
								}
							}
							else
							{
								hr = E_POINTER; // or any appropriate error code
								ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
							}
						}
						else if (fileFormat.compare(L"exr") == 0)
						{
							// Requesting a .exr file
							if (buffer != nullptr)
							{
								ScratchImage scratch = {};
								hr = LoadFromEXRFile(name.c_str(), nullptr, scratch);

								if (FAILED(hr))
								{
									ref->client->Con_Printf(PRINT_ALL, L"Failed to load EXR: " + name + L".");
								}
								else
								{
									currentWidth = scratch.GetMetadata().width;
									currentHeight = scratch.GetMetadata().height;
									format = scratch.GetMetadata().format;

									auto result = GetSubresourcesFromScratch(scratch);

									if (result.size() >= 1)
									{
										subresources[i] = result[0];
									}
								}
							}
							else
							{
								hr = E_POINTER; // or any appropriate error code
								ref->client->Con_Printf(PRINT_ALL, L"Buffer is null for " + name + L".");
							}
						}
						else
						{
							// Assume a WIC compatible format (.bmp, .jpg, .png, .tif, .gif, .ico, .wdp, .jxr)
							m_resourceUploadBatch->Begin();

							ComPtr<ID3D12Resource> tempResource = nullptr;
							hr = CreateWICTextureFromMemory(
								ref->sys->dx->Device().Get(),
								*m_resourceUploadBatch,
								buffer,
								bufferSize,
								tempResource.ReleaseAndGetAddressOf(),
								false,
								0);

							if (FAILED(hr))
							{
								ref->client->Con_Printf(PRINT_ALL, L"Failed to load " + name + L" with WIC loader.");
							}

							// Upload the resources to the GPU.
							auto finish = m_resourceUploadBatch->End(ref->sys->dx->m_commandQueue.Get());

							// Wait for the upload thread to terminate
							finish.wait();

							if (tempResource)
							{
								currentWidth = tempResource->GetDesc().Width;
								currentHeight = tempResource->GetDesc().Height;
								format = tempResource->GetDesc().Format;

								auto result = GetSubresourcesFromResource(tempResource);

								if (result.size() >= 1)
								{
									subresources[i] = result[0];
								}

								SAFE_RELEASE(tempResource);
							}
						}

						if (i == 0)
						{
							width = currentWidth;
							height = currentHeight;
						}

						if ((width != currentWidth) || (height != currentHeight))
						{
							LOG(warning) << "Invalid cubemap face " << name + suffixes[i] << " dimensions " << currentWidth << "x" << currentHeight << " versus " << width << "x" << height << "!";

							// Free allocated memory
							for (int j = 0; j <= i; ++j)
							{
								/*if (subresources[j].pData)
								{
									delete[] subresources[j].pData;
									subresources[j].pData = nullptr;
								}*/
								validCubeData = false;
							}
						}
						else if ((currentWidth > 0) && (currentHeight > 0))
						{
							if (subresources[i].pData)
							{
								totalSliceSize += subresources[i].SlicePitch;
							}
						}
					}
				}
			}

			if (!validCubeData)
			{
				// Create a default rainbow cube
				const size_t cubeEdgePixels = 16;
				const size_t bytesPerPixel = 4; // RGBA
				const size_t faceSize = cubeEdgePixels * cubeEdgePixels * bytesPerPixel;

				// Allocate memory for each face's data
				static BYTE faceData[faceSize * 6];

				BYTE colors[6][4] = {
					{ 255, 0, 0, 255 },   // Red
					{ 0, 255, 0, 255 },   // Green
					{ 0, 0, 255, 255 },   // Blue
					{ 255, 255, 0, 255 }, // Yellow
					{ 0, 255, 255, 255 }, // Cyan
					{ 255, 0, 255, 255 }  // Magenta
				};

				for (size_t face = 0; face < 6; face++)
				{
					byte* currentFaceData = faceData + face * faceSize;

					for (size_t y = 0; y < cubeEdgePixels; ++y)
					{
						for (size_t x = 0; x < cubeEdgePixels; ++x)
						{
							size_t index = (y * cubeEdgePixels + x) * bytesPerPixel;

							currentFaceData[index]     = colors[face][0]; // Red
							currentFaceData[index + 1] = colors[face][1]; // Green
							currentFaceData[index + 2] = colors[face][2]; // Blue
							currentFaceData[index + 3] = colors[face][3]; // Alpha
						}
					}

					subresources[face].pData = currentFaceData;
					subresources[face].RowPitch = cubeEdgePixels * bytesPerPixel;
					subresources[face].SlicePitch = subresources[face].RowPitch * cubeEdgePixels;
				}

				width = height = cubeEdgePixels;
				format = DXGI_FORMAT_R8G8B8A8_UNORM;
				validCubeData = true;
			}

			if (validCubeData)
			{
				cubemap = CreateTexture(name + L"_sky", it_sky, width, height, false, format, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 6);
				cubemap->m_subresources = subresources;

				UpdateTexture(cubemap);
			}

			// Cleanup the individual subresources
			/*for (int i = 0; i < 6; ++i)
			{
				if (subresources[i].pData)
				{
					delete[] subresources[i].pData;
					subresources[i].pData = nullptr;
				}
			}*/

			if (cubemap)
			{
				cubemap->m_cubemap = true;
				cubemap->m_registrationSequence = ref->media->RegistrationSequence();

				return cubemap;
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return nullptr;
}

void dx12::ImageManager::SetRawPalette(const unsigned char* palette)
{
	LOG_FUNC();

	try
	{
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
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

D3D12_SUBRESOURCE_DATA dx12::ImageManager::DepalletizeRawData(size_t width, size_t height, unsigned int bpp, byte* raw, XMCOLOR* palette)
{
	LOG_FUNC();

	D3D12_SUBRESOURCE_DATA		outData = {};
	HRESULT hr = E_UNEXPECTED;

	try {
		ZeroMemory(&outData, sizeof(D3D12_SUBRESOURCE_DATA));

		if (raw != nullptr)
		{
			outData.RowPitch = width * (sizeof(unsigned int) / sizeof(byte));
			outData.SlicePitch = width * height * (sizeof(unsigned int) / sizeof(byte));
			unsigned int* rgba32 = nullptr;

			if (bpp == BPP_8)
			{
				rgba32 = new unsigned int[width * height]();
				// De-paletteize the texture data

				//for (unsigned int i = 0; i < (width * height); i++)
				Concurrency::parallel_for<size_t>(0, (width * height), [&raw, &rgba32, &palette](size_t i)
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
				Concurrency::parallel_for<size_t>(0, (width * height), [&raw, &rgba32](size_t i)
					{
						size_t index = i * 3;
						rgba32[i] = (raw[index] << 24u | raw[index + 1] << 16u | raw[index + 2] << 8u | 255u);
					});
			}
			else if (bpp == BPP_32)
			{
				//std::memcpy(&rgba32, raw, width * height * (sizeof(unsigned int) / sizeof(byte)));
				rgba32 = reinterpret_cast<unsigned int*>(raw);
			}

			outData.pData = rgba32;
		}

	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return outData;
}

static UINT GetBitsPerPixel(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return 64;

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return 32;

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return 32;

	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
		return 32;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		return 32;

	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return 32;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
		return 16;

	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return 32;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		return 32;

	case DXGI_FORMAT_B5G6R5_UNORM:
		return 16;

	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;

	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 24;

	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_Y416:
		return 32;

	case DXGI_FORMAT_YUY2:
		return 16;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 20;

		//case DXGI_FORMAT_NV21:
		//	return 12;

		//case DXGI_FORMAT_P210:
		//case DXGI_FORMAT_P216:
		//	return 24;

		//case DXGI_FORMAT_AYUV_10B:
	case DXGI_FORMAT_P8:
	case DXGI_FORMAT_A8P8:
		return 8;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
}


bool dx12::ImageManager::UploadSubresources(std::shared_ptr<dx12::Texture> texture)
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	try
	{
		if (m_resourceUploadBatch)
		{
			m_resourceUploadBatch->Begin();

			/*if (commandList && commandList->IsOpen())
			{
				auto uploadHeap = ref->res->GetOrCreateResource<Resource>(L"UploadHeap_" + texture->GetName());

				// Define heap properties for an upload buffer
				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;  // Must be UPLOAD type for CPU -> GPU transfers

				// Describe the upload buffer
				uploadHeap->m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				uploadHeap->m_resourceDesc.Width = GetBitsPerPixel(texture->m_resourceDesc.Format) * static_cast<UINT64>(texture->m_resourceDesc.Width) * texture->m_resourceDesc.Height;
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
					return false;
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
					texture->m_subresources.size(),
					texture->m_subresources.data()
				);

				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				commandList->List()->ResourceBarrier(1, &barrier);

				texture->m_srvHandle = 0;
				texture->m_cachedD3D12SRVHandle = {};
			}
			else*/
			{
				m_resourceUploadBatch->Transition(
					texture->m_resource.Get(),
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_COPY_DEST);

				m_resourceUploadBatch->Upload(texture->m_resource.Get(), 0, texture->m_subresources.data(), msl::utilities::SafeInt<uint32_t>(texture->m_subresources.size()));

				m_resourceUploadBatch->Transition(
					texture->m_resource.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}

			// Generate mipmaps
			if ((texture->m_resourceDesc.MipLevels == 0) &&
				(texture->m_resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D) &&
				(texture->m_resourceDesc.DepthOrArraySize == 1))
			{
				LOG(debug) << "Generating mipmaps for texture " << texture->m_name;
				texture->m_resourceDesc.MipLevels = 1;
				m_resourceUploadBatch->GenerateMips(texture->m_resource.Get());
			}

			auto uploadResourcesFinished = m_resourceUploadBatch->End(ref->sys->dx->m_commandQueue.Get());

			uploadResourcesFinished.wait();

			texture->m_resource->SetName(texture->GetName().c_str());

			return true;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

byte* dx12::ImageManager::LoadImageFile(std::wstring name, imagetype_t type, std::wstring &fileFormat, size_t& bufferSize)
{
	LOG_FUNC();

	try {
		byte* buffer = nullptr;

		// Determine the image type
		std::wstring fileName = std::filesystem::path(name).stem().wstring();
		std::wstring extension = std::filesystem::path(name).extension().wstring();
		std::wstring path = L"";

		if ((fileName.size() > 0) && (fileName.at(0) != '/') && (fileName.at(0) != '\\'))
		{
			switch (type)
			{
			case it_pic:
				path = L"pics/";
				break;
			case it_skin:
				path = std::filesystem::path(name).parent_path().wstring() + L"/";
				break;
			case it_sprite:
				path = L"sprites/";
				break;
			case it_wall:
				path = std::filesystem::path(name).parent_path().wstring() + L"/";
				break;
			case it_sky:
				path = L"env/";
				break;
			default:
				break;
			}
		}
		else
		{
			fileName = name.substr(1, name.length() - 1);
		}

		// Iterate through all possible format extensions until we find the file
		for (std::wstring format : m_imageExtensions)
		{
			buffer = nullptr;
			bufferSize = 0;

			// Attempt to load the file
			bufferSize = ref->client->FS_LoadFile(path + fileName + L"." + format, reinterpret_cast<void**>(&buffer));

			if ((bufferSize > 0) && (buffer))
			{
				fileFormat = format;
				return buffer;
			}
		}

		if (bufferSize <= 0)
		{
			ref->client->Con_Printf(PRINT_ALL, L"File not found: " + name);
			return nullptr;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	ref->client->Con_Printf(PRINT_ALL, L"File not found: " + name);
	return nullptr;
}

void dx12::ImageManager::ClearSubresources(std::shared_ptr<dx12::Texture> texture, size_t size)
{
	LOG_FUNC();

	try {
		texture->m_subresources.clear();

		if (size > 0)
		{
			texture->m_subresources.resize(size);

			for (unsigned int i = 0; i < size; i++)
			{
				ZeroMemory(&texture->m_subresources[i], sizeof(D3D12_SUBRESOURCE_DATA));
			}
		}
		else
		{
			texture->m_subresources.shrink_to_fit();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
