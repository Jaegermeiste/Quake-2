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

void dx11::Draw::GetPicSize(unsigned int & w, unsigned int & h, std::string name)
{
	std::shared_ptr<dx11::Texture2D> image = ref->media->img->Load(name, it_pic);

	if (image)
	{
		w = image->m_data->m_textureDesc.Width;
		h = image->m_data->m_textureDesc.Height;
	}
}

void dx11::Draw::Pic(int x, int y, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->media->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, msl::utilities::SafeInt<int>(image->m_data->m_textureDesc.Width), msl::utilities::SafeInt<int>(image->m_data->m_textureDesc.Height), DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_data->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::StretchPic(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->media->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_data->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::Char(int x, int y, unsigned char c)
{
	LOG_FUNC();

	if ((y <= -SMALL_CHAR_SIZE) || ((c & 127) == 32))
	{
		// Offscreen, or space
		return;
	}

	unsigned char row = static_cast<unsigned char>(c >> 4);
	unsigned char col = static_cast<unsigned char>(c & 15);

	float size = 0.0625f;
	float frow = row * size;
	float fcol = col * size;

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, fcol, frow, fcol + size, frow + size, DirectX::Colors::White);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->media->img->m_conChars->m_data->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);

	/*

	if ((c >= '!') && (c <= '}'))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = c;

		// Gray Text
		//ref->sys->dx->subsystemText->RenderText(x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorGray);
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::Gray);

		// Render the overlay to the back buffer
		ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
	else if ((c >= 161) && (c <= 253))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = msl::utilities::SafeInt<unsigned char>(c - 128);

		// Green Text
		//ref->sys->dx->subsystemText->RenderText(x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorYellowGreen);
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::Green);

		// Render the overlay to the back buffer
		ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
	}*/
}

void dx11::Draw::TileClear(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->media->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_data->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::Fill(int x, int y, int w, int h, int c)
{
	LOG_FUNC();

	if (msl::utilities::SafeInt<unsigned int>(c) > 255)
	{
		ref->client->Sys_Error(ERR_FATAL, "Draw_Fill: bad color");
		return;
	}

	DirectX::XMVECTORF32 color = DirectX::Colors::White;
	color.f[0] = ref->media->img->m_8to32table[c].r;
	color.f[1] = ref->media->img->m_8to32table[c].g;
	color.f[2] = ref->media->img->m_8to32table[c].b;

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, color);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
}

void dx11::Draw::FadeScreen(void)
{
	//ref->sys->dx->subsystem2D->FadeScreen();

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(0, 0, msl::utilities::SafeInt<int>(ref->sys->dx->subsystem2D->m_renderTargetWidth), msl::utilities::SafeInt<int>(ref->sys->dx->subsystem2D->m_renderTargetHeight), { 0.0f, 0.0f, 0.0f, 0.75f });

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
}

void dx11::Draw::StretchRaw(int x, int y, int w, int h, unsigned int cols, unsigned int rows, byte * data)
{
	LOG_FUNC();

	if ((x < 0) || (x + w > ref->sys->dx->m_windowWidth) || (y + h > ref->sys->dx->m_windowHeight))
	{
		ref->client->Sys_Error(ERR_FATAL, "Draw_StretchRaw: bad coordinates");
	}
	
	unsigned int	*image32 = new unsigned int[rows * cols]();
	DirectX::PackedVector::XMCOLOR* palette = ref->media->img->m_rawPalette;

	// De-palletize the texture data
	Concurrency::parallel_for(0u, (rows * cols), [&data, &image32, &palette](unsigned int i)
	{
		// Paletted
		image32[i] = palette[data[i]];
	});

	if (!ref->media->img->m_rawTexture)
	{
		// Create the texture on demand
		ref->media->img->m_rawTexture = ref->media->img->CreateTexture2DFromRaw("StretchRaw", cols, rows, false, BPP_32, (byte*)image32, nullptr, D3D11_USAGE_DYNAMIC);
	}
	else
	{
		// Update the texture
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//	Disable GPU access to the texture data.
		ref->sys->dx->subsystem2D->m_2DdeferredContext->Map(ref->media->img->m_rawTexture->m_data->m_resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		
		//	Update the texture here.
		memcpy(mappedResource.pData, image32, sizeof(unsigned int) * rows * cols);

		//	Reenable GPU access to the texture data.
		ref->sys->dx->subsystem2D->m_2DdeferredContext->Unmap(ref->media->img->m_rawTexture->m_data->m_resource, 0);
	}
	
	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

	ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->media->img->m_rawTexture->m_data->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);

	if (image32)
	{
		delete[] image32;
		image32 = nullptr;
	}
}
