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

void dx12::Draw::GetPicSize(unsigned int & w, unsigned int & h, std::wstring name)
{
	try
	{
		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(name, it_pic);

		if (image)
		{
			w = image->GetWidth();
			h = image->GetHeight();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::Pic(int x, int y, std::wstring name)
{
	LOG_FUNC();

	try
	{
		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(name, it_pic);

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (image)
		{
			image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, msl::utilities::SafeInt<int>(image->GetWidth()), msl::utilities::SafeInt<int>(image->GetHeight()), DirectX::Colors::White);

			m_quadIndex++;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::StretchPic(int x, int y, int w, int h, std::wstring name)
{
	LOG_FUNC();

	try
	{
		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(name, it_pic);

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (image)
		{
			image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, w, h, DirectX::Colors::White);

			m_quadIndex++;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::Char(int x, int y, unsigned char c)
{
	LOG_FUNC();

	try
	{
		c &= 255;

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

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (ref->media->img->m_conChars)
		{
			ref->media->img->m_conChars->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, fcol, frow, fcol + size, frow + size, DirectX::Colors::White);

			m_quadIndex++;
		}

		// Render the overlay to the back buffer
		//ref->sys->dx->subsystem2D->m_2DshaderTexture.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->media->img->m_conChars->m_shaderResourceView, ref->sys->dx->subsystem2D->m_constantBuffer);

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
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::TileClear(int x, int y, int w, int h, std::wstring name)
{
	LOG_FUNC();

	try
	{
		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(name, it_pic);

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (image)
		{
			image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, w, h, x / 64.0f, y / 64.0f, (x + w) / 64.0f, (y + h) / 64.0f, DirectX::Colors::White);

			m_quadIndex++;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::Fill(int x, int y, int w, int h, int c)
{
	LOG_FUNC();

	try
	{
		if (msl::utilities::SafeInt<unsigned int>(c) > 255)
		{
			ref->client->Sys_Error(ERR_FATAL, L"Draw_Fill: bad color");
			return;
		}

		DirectX::XMVECTORF32 color = DirectX::Colors::White;
		color.f[0] = ref->media->img->m_8to32table[c].r;
		color.f[1] = ref->media->img->m_8to32table[c].g;
		color.f[2] = ref->media->img->m_8to32table[c].b;

		std::shared_ptr<dx12::Texture> image = ref->media->img->Load(WHITE_TEXTURE_NAME, it_pic);

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (image)
		{
			image->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, w, h, DirectX::Colors::White);

			m_quadIndex++;
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::FadeScreen(void)
{
	LOG_FUNC();

	try
	{
		ref->sys->dx->subsystem2D->FadeScreen();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::StretchRaw(int x, int y, int w, int h, unsigned int cols, unsigned int rows, byte * data)
{
	LOG_FUNC();

	try
	{
		if ((x < 0) || (x + w > ref->sys->dx->m_modeWidth) || (y + h > ref->sys->dx->m_modeHeight))
		{
			ref->client->Sys_Error(ERR_FATAL, L"Draw_StretchRaw: bad coordinates");
		}

		if (!ref->media->img->m_rawTexture)
		{
			// Create the texture on demand
			ref->media->img->m_rawTexture = ref->media->img->CreateTextureFromRaw(L"StretchRaw", cols, rows, false, BPP_8, data, ref->media->img->m_rawPalette);
		}
		else
		{
			ref->media->img->UpdateTextureFromRaw(ref->media->img->m_rawTexture, cols, rows, false, BPP_8, data, ref->media->img->m_rawPalette);
		}

		if (m_quadIndex >= m_quadList.capacity())
		{
			auto oldCapacity = m_quadList.capacity();

			LOG(warning) << "Growing Quad list...";
			m_quadList.resize(oldCapacity * 2);

			for (int i = m_quadIndex; i < m_quadList.capacity(); ++i) {
				m_quadList.at(i).Initialize(0, 0, SMALL_CHAR_SIZE, SMALL_CHAR_SIZE, DirectX::Colors::White);
			}
		}

		if (ref->media->img->m_rawTexture)
		{
			ref->media->img->m_rawTexture->BindSRV(ref->sys->dx->subsystem2D->m_commandList);

			m_quadList.at(m_quadIndex).Render(ref->sys->dx->subsystem2D->m_commandList, x, y, w, h, DirectX::Colors::White);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::Initialize()
{
	try
	{
		for (auto& quad : m_quadList) {
			quad.Initialize(0, 0, 128, 128, DirectX::Colors::White);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

void dx12::Draw::Shutdown()
{
	try
	{
		for (auto& quad : m_quadList) {
			quad.Shutdown();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}
