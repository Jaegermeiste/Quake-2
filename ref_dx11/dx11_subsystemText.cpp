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

dx11::SubsystemText::SubsystemText()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	m_writeFactory = nullptr;
	m_textFormat = nullptr;
}

bool dx11::SubsystemText::Initialize()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_writeFactory)
	);

	if (FAILED(hr))
	{
		LOG(error) << "Failed to create write factory.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created write factory.";
	}

	hr = m_writeFactory->CreateTextFormat(
		ref->sys->convertUTF.from_bytes(ref->cvars->font->String().c_str()).c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		9,
		L"en-us",
		&m_textFormat
	);

	if (FAILED(hr))
	{
		LOG(error) << "Failed to create text format.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created text format.";
	}

	// Center the text horizontally and vertically.
	m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);


	return true;
}

void dx11::SubsystemText::RenderText(int x, int y, int w, int h, WCHAR* text, ID2D1SolidColorBrush* colorBrush)
{
	LOG_FUNC();

	ref->sys->dx->subsystem2D->ActivateD2DDrawing();

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->BeginDraw();

	//ref->sys->dx->m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	//ref->sys->dx->m_d2dContext->DrawText(
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->DrawText(
		text,
		wcslen(text),
		m_textFormat,
		D2D1::RectF(x, y, x + w, y + h),
		colorBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->EndDraw();

}

void dx11::SubsystemText::RenderText(int x, int y, int w, int h, std::string text, ID2D1SolidColorBrush* colorBrush)
{
	LOG_FUNC();

	ref->sys->dx->subsystem2D->ActivateD2DDrawing();
	
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->BeginDraw();

	//ref->sys->dx->m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	//ref->sys->dx->subsystem2D->m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	
	//ref->sys->dx->m_d2dContext->DrawText(
	ref->sys->dx->subsystem2D->m_d2dRenderTarget->DrawText(
		ref->sys->convertUTF.from_bytes(text.c_str()).c_str(),
		text.length(),
		m_textFormat,
		D2D1::RectF(x, y,x + w, y + h),
		colorBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);

	ref->sys->dx->subsystem2D->m_d2dRenderTarget->EndDraw();

}

void dx11::SubsystemText::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	SAFE_RELEASE(m_textFormat);

	SAFE_RELEASE(m_writeFactory);

	LOG(info) << "Shutdown complete.";
}
