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
	std::shared_ptr<dx11::Texture2D> image = ref->img->Load(name, it_pic);

	if (image)
	{
		w = image->m_textureDesc.Width;
		h = image->m_textureDesc.Height;
	}
}

void dx11::Draw::Pic(int x, int y, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, msl::utilities::SafeInt<int>(image->m_textureDesc.Width), msl::utilities::SafeInt<int>(image->m_textureDesc.Height), DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::StretchPic(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::Char(int x, int y, unsigned char c)
{
	LOG_FUNC();

	if (y <= -8)
		return;			// totally off screen

	if ((c & 127) == 32)
		return;		// space

	if ((c >= '!') && (c <= '}'))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = c;

		// Gray Text
		//ref->sys->dx->subsystemText->RenderText(x, y, CHAR_SIZE, CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorGray);
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, CHAR_SIZE, CHAR_SIZE, DirectX::Colors::Gray);

		// Render the overlay to the back buffer
		ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
	else if ((c >= 161) && (c <= 253))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = msl::utilities::SafeInt<unsigned char>(c - 128);

		// Green Text
		//ref->sys->dx->subsystemText->RenderText(x, y, CHAR_SIZE, CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorYellowGreen);
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, CHAR_SIZE, CHAR_SIZE, DirectX::Colors::Green);

		// Render the overlay to the back buffer
		ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, nullptr, ref->sys->dx->subsystem2D->m_constantBuffer);
	}
}

void dx11::Draw::TileClear(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	std::shared_ptr<dx11::Texture2D> image = ref->img->Load(name, it_pic);

	if (image)
	{
		ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

		ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, image->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);
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
	color.f[0] = ref->img->m_8to32table[c].r;
	color.f[1] = ref->img->m_8to32table[c].g;
	color.f[2] = ref->img->m_8to32table[c].b;

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
	/*
	unsigned int	image32[256 * 256];
	ZeroMemory(&image32, sizeof(unsigned int) * 256 * 256);
	int				trows = rows;
	byte			*source = nullptr;
	int				frac, fracstep;
	float			hscale = 1.0f;
	int				row = 0;
	float			t;
	
	if (rows <= 256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows / 256.0;
		trows = 256;
	}
	t = rows * hscale / 256;

	for (unsigned int i = 0; i < trows; i++)
	{
		row = static_cast<int>(i * hscale);
		if (row > rows)
		{
			break;
		}

		source = data + cols * row;
		unsigned int *dest = &image32[i * 256];
		fracstep = cols * 0x10000 / 256;
		frac = fracstep >> 1;

		for (unsigned int j = 0; j < 256; j++)
		{
			dest[j] = ref->img->m_rawPalette[source[frac >> 16]];
			frac += fracstep;
		}
	}
	
	byte* image = reinterpret_cast<byte*>(&image32);

	auto texture = ref->img->CreateTexture2DFromRaw("StretchRaw", cols, rows, false, BPP_32, image, nullptr);*/

	auto texture = ref->img->CreateTexture2DFromRaw("StretchRaw", cols, rows, false, BPP_8, data, ref->img->m_rawPalette);
	
	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::White);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, texture->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);

	// Destroy the texture
	SAFE_RELEASE(texture->m_shaderResourceView);
	SAFE_RELEASE(texture->m_texture2D);
	SAFE_RELEASE(texture->m_resource);
	ZeroMemory(&texture->m_textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texture->m_registrationSequence = 0;
	texture->m_name.clear();
	texture->m_format.clear();
}
