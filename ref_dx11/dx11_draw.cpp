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
}

void dx11::Draw::Pic(int x, int y, std::string name)
{
	LOG_FUNC();

	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	//ref->sys->dx->m_d2dContext->FillRectangle(D2D1::RectF(x, y, CHAR_SIZE, CHAR_SIZE), ref->sys->dx->subsystem2D->colorBlack);
}

void dx11::Draw::StretchPic(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	//ref->sys->dx->m_d2dContext->FillRectangle(D2D1::RectF(x, y, x + w, y + h), ref->sys->dx->subsystem2D->colorBlack);

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::Linen);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->sys->dx->subsystem2D->m_2DshaderResourceView);
}

void dx11::Draw::Char(int x, int y, unsigned char c)
{
	LOG_FUNC();

	if (y <= -8)
		return;			// totally off screen

	
	if ((c >= '!') && (c <= '}'))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = c;

		// Gray Text
		//ref->sys->dx->subsystemText->RenderText(x, y, CHAR_SIZE, CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorGray);
	}
	else if ((c >= 161) && (c <= 253))
	{
		WCHAR		strChar[2];
		ZeroMemory(&strChar, sizeof(WCHAR) * 2);
		strChar[0] = (c - 128);

		// Green Text
		//ref->sys->dx->subsystemText->RenderText(x, y, CHAR_SIZE, CHAR_SIZE, strChar, ref->sys->dx->subsystem2D->colorYellowGreen);
	}
}

void dx11::Draw::TileClear(int x, int y, int w, int h, std::string name)
{
	LOG_FUNC();

	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	//ref->sys->dx->m_d2dContext->FillRectangle(D2D1::RectF(x, y, x + w, y + h), ref->sys->dx->subsystem2D->colorBlack);

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::Linen);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->sys->dx->subsystem2D->m_2DshaderResourceView);
}

void dx11::Draw::Fill(int x, int y, int w, int h, int c)
{
	LOG_FUNC();

	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	//ref->sys->dx->m_d2dContext->FillRectangle(D2D1::RectF(x, y, x + w, y + h),ref->sys->dx->subsystem2D->colorBlack);

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::Linen);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->sys->dx->subsystem2D->m_2DshaderResourceView);
}

void dx11::Draw::FadeScreen(void)
{
	//ref->sys->dx->subsystem2D->FadeScreen();

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(0, 0, ref->sys->dx->subsystem2D->m_renderTargetWidth, ref->sys->dx->subsystem2D->m_renderTargetHeight, { 0.0f, 0.0f, 0.0f, 0.75f });

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->sys->dx->subsystem2D->m_2DshaderResourceView);
}

void dx11::Draw::StretchRaw(int x, int y, int w, int h, unsigned int cols, unsigned int rows, byte * data)
{
	LOG_FUNC();

	//ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	//ref->sys->dx->m_d2dContext->FillRectangle(D2D1::RectF(x, y, x + w, y + h), ref->sys->dx->subsystem2D->colorBlack);

	ref->sys->dx->subsystem2D->m_generalPurposeQuad.Render(x, y, w, h, DirectX::Colors::Linen);

	// Render the overlay to the back buffer
	ref->sys->dx->subsystem2D->m_2DshaderVertexColor.Render(ref->sys->dx->subsystem2D->m_2DdeferredContext, ref->sys->dx->subsystem2D->m_generalPurposeQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), ref->sys->dx->subsystem2D->m_2DorthographicMatrix, ref->sys->dx->subsystem2D->m_2DshaderResourceView);
}
