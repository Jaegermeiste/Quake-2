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

#define DEFERRED	1

dx11::Subsystem2D::Subsystem2D()
{
	LOG_FUNC();

	LOG(info) << "Initializing";

	m_2DdeferredContext = nullptr;
	m_2DcommandList = nullptr;
	m_2DrenderTargetTexture = nullptr;
	m_2DoverlayRTV = nullptr;
	m_2DshaderResourceView = nullptr;
	ZeroMemory(&m_2DorthographicMatrix, sizeof(DirectX::XMMATRIX));
	m_depthDisabledStencilState = nullptr;
	m_dxgiSurface = nullptr;
	m_d2dRenderTarget = nullptr;
	m_d2dDrawingActive = false;
	m_2DunorderedAccessView = nullptr;
	m_alphaBlendState = nullptr;

	fadeColor = nullptr;
	colorBlack = nullptr;
	colorGray = nullptr;
	colorYellowGreen = nullptr;
}

/*
http://rastertek.com/dx11tut11.html
*/
bool dx11::Subsystem2D::Initialize()
{
	LOG_FUNC();

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr = E_UNEXPECTED;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;

	// Wipe Structs
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	ZeroMemory(&depthDisabledStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ZeroMemory(&unorderedAccessViewDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	// Set modified for first run
	ref->cvars->overlayScale->SetModified(true);

	// Set width and height
	m_renderTargetWidth = ref->sys->dx->m_windowWidth;
	m_renderTargetHeight = ref->sys->dx->m_windowHeight;

#ifdef DEFERRED
	// Create deferred context
	hr = ref->sys->dx->m_d3dDevice->CreateDeferredContext(0, &m_2DdeferredContext);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create 2D deferred context.";
		return false;
	}
#else
	m_2DdeferredContext = ref->sys->dx->m_immediateContext;
#endif

	// Setup the render target texture description.
	textureDesc.Width = m_renderTargetWidth;
	textureDesc.Height = m_renderTargetHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	/*
	// Find a format that D2D is happy with
	if (ref->sys->dx->m_d2dContext->IsDxgiFormatSupported(DXGI_FORMAT_R32G32B32A32_FLOAT))
	{
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		LOG(info) << "Overlay texture format: DXGI_FORMAT_R32G32B32A32_FLOAT";
	}
	else if (ref->sys->dx->m_d2dContext->IsDxgiFormatSupported(DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		LOG(info) << "Overlay texture format: DXGI_FORMAT_R16G16B16A16_FLOAT";
	}
	else if (ref->sys->dx->m_d2dContext->IsDxgiFormatSupported(DXGI_FORMAT_R16G16B16A16_UNORM))
	{
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		LOG(info) << "Overlay texture format: DXGI_FORMAT_R16G16B16A16_UNORM";
	}
	else if (ref->sys->dx->m_d2dContext->IsDxgiFormatSupported(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB))
	{
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		LOG(info) << "Overlay texture format: DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
	}
	else
	{
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		LOG(info) << "Overlay texture format: DXGI_FORMAT_B8G8R8A8_UNORM";
	}*/
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	// Create the render target texture.
	hr = ref->sys->dx->m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_2DrenderTargetTexture);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create Texture2D.";
		return false;
	}

	hr = m_2DrenderTargetTexture->QueryInterface(&m_dxgiSurface);

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create DXGISurface.";
		return false;
	}

/*	// Create a D2D render target which can draw into our offscreen D3D
	// surface. Given that we use a constant size for the texture, we
	// fix the DPI at 96.
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_HARDWARE,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			0, 0);

	hr = ref->sys->dx->m_d2dFactory->CreateDxgiSurfaceRenderTarget(m_dxgiSurface, &props, &m_d2dRenderTarget);

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create D2D render target.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created D2D render target.";
	}

	ref->sys->dx->m_d2dContext->BeginDraw();*/

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	hr = ref->sys->dx->m_d3dDevice->CreateRenderTargetView(m_2DrenderTargetTexture, &renderTargetViewDesc, &m_2DoverlayRTV);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create overlay RenderTargetView.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created overlay RenderTargetView.";
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = ref->sys->dx->m_d3dDevice->CreateShaderResourceView(m_2DrenderTargetTexture, &shaderResourceViewDesc, &m_2DshaderResourceView);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create ShaderResourceView.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created ShaderResourceView.";
	}

	/*// Setup the description of the unordered access view.
	unorderedAccessViewDesc.Format = textureDesc.Format;
	unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	unorderedAccessViewDesc.Texture2D.MipSlice = 0;

	// Create the unordered access view.
	hr = ref->sys->dx->m_d3dDevice->CreateUnorderedAccessView(m_2DrenderTargetTexture, &unorderedAccessViewDesc, &m_2DunorderedAccessView);
	if (FAILED(hr))
	{
		LOG(error) << "Unable to create overlay UnorderedAccessView.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created overlay UnorderedAccessView.";
	}*/

	// Set the overlay RTV as the current render target
	m_2DdeferredContext->OMSetRenderTargets(1, &m_2DoverlayRTV, nullptr);

	//ref->sys->dx->subsystem2D->m_2DdeferredContext->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 1, &m_2DunorderedAccessView);

	// Create an orthographic projection matrix for 2D rendering.
	m_2DorthographicMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(m_renderTargetWidth), static_cast<float>(m_renderTargetHeight), ref->cvars->zNear2D->Float(), ref->cvars->zFar2D->Float());

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	hr = ref->sys->dx->m_d3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create DepthStencilState.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created DepthStencilState.";
	}


	D3D11_BLEND_DESC bs;
	ZeroMemory(&bs, sizeof(D3D11_BLEND_DESC));
	for (int i = 0; i<8; i++)
	{
		bs.RenderTarget[i].BlendEnable = TRUE;
		bs.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		bs.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bs.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		bs.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		bs.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		bs.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		bs.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
	}
	bs.IndependentBlendEnable = TRUE;

	hr = ref->sys->dx->m_d3dDevice->CreateBlendState(&bs, &m_alphaBlendState);

	if (FAILED(hr))
	{
		LOG(error) << "Unable to create BlendState.";
		return false;
	}
	else
	{
		LOG(info) << "Successfully created BlendState.";
	}

	ref->sys->dx->subsystem2D->m_2DdeferredContext->OMSetBlendState(m_alphaBlendState, NULL, 1u);

	// Setup the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (FLOAT)m_renderTargetWidth;
	viewport.Height = (FLOAT)m_renderTargetHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	LOG(info) << "Setting viewport.";

	m_2DdeferredContext->RSSetViewports(1, &viewport);

	UINT viewportCount = 0;
	m_2DdeferredContext->RSGetViewports(&viewportCount, nullptr);
	LOG(info) << std::to_string(viewportCount) << " viewports bound.";

	// Calculate the width of the overlay.
	int width = static_cast<int>(static_cast<float>(m_renderTargetWidth) * ref->cvars->overlayScale->Float());

	// Calculate the screen coordinates of the left side of the overlay.
	int x = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowWidth) - width) / 2);

	// Calculate the height of the overlay.
	int height = static_cast<int>(static_cast<float>(m_renderTargetHeight) * ref->cvars->overlayScale->Float());

	// Calculate the screen coordinates of the top of the overlay.
	int y = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowHeight) - height) / 2);

	if (!m_renderTargetQuad.Initialize(ref->sys->dx->m_immediateContext, x, y, width, height, DirectX::Colors::White))
	{
		LOG(error) << "Failed to properly initialize render target quad.";
		return false;
	}

	if (!m_generalPurposeQuad.Initialize(ref->sys->dx->subsystem2D->m_2DdeferredContext, x, y, width, height, DirectX::Colors::White))
	{
		LOG(error) << "Failed to properly initialize general purpose quad.";
		return false;
	}

	if (!m_2DshaderVertexColor.Initialize(ref->sys->dx->m_d3dDevice, "colorVertex.hlsl", "vertexPixel.hlsl"))
	{
		LOG(error) << "Failed to properly create shaders.";
		return false;
	}

	if (!m_2DshaderTexture.Initialize(ref->sys->dx->m_d3dDevice, "simpleVertex.hlsl", "simplePixel.hlsl"))
	{
		LOG(error) << "Failed to properly create shaders.";
		return false;
	}


/*
	if (SUCCEEDED(hr))
	{
		hr = m_d2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black, 0.8f),
			&fadeColor
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_d2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
			&colorBlack
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_d2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Gray, 1.0f),
			&colorGray
		);
	}

	// Create a solid color brush with its rgb value 0x9ACD32.
	if (SUCCEEDED(hr))
	{
		hr = m_d2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0x9ACD32, 1.0f)),
			&colorYellowGreen
		);
	}
	*/
	LOG(info) << "Successfully initialized 2D subsystem.";

	return true;
}

void dx11::Subsystem2D::ActivateD2DDrawing()
{
	LOG_FUNC();

	if ((ref->sys->dx->m_d2dContext) && (!m_d2dDrawingActive))
	{
		//LOG(trace) << "Beginning D2D Draw";

		ref->sys->dx->m_d2dContext->BeginDraw();
		m_d2dDrawingActive = true;
	}
}

void dx11::Subsystem2D::EndD2DDrawing()
{
	LOG_FUNC();

	if ((ref->sys->dx->m_d2dContext) && (m_d2dDrawingActive))
	{
		HRESULT hr = E_UNEXPECTED;

		//LOG(info) << "Ending D2D Draw";

		hr = ref->sys->dx->m_d2dContext->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			LOG(error) << "m_d2dContext->EndDraw() returned D2DERR_RECREATE_TARGET";
		}
		m_d2dDrawingActive = false;
	}
}



void dx11::Subsystem2D::Clear()
{
	LOG_FUNC();

	// Clear 2D deferred
	if (ref->sys->dx->m_immediateContext)
	{
		LOG(trace) << "Clearing overlay RenderTargetView.";

		// Clear the GUI Overlay buffer to transparent
#ifndef _DEBUG
		m_2DdeferredContext->ClearRenderTargetView(m_2DoverlayRTV, DirectX::Colors::Transparent);
#else
		m_2DdeferredContext->ClearRenderTargetView(m_2DoverlayRTV, DirectX::Colors::Red);
#endif
	}
}

void dx11::Subsystem2D::Render()
{
	LOG_FUNC();

	HRESULT hr = E_UNEXPECTED;

	if (ref->sys->dx->m_immediateContext)
	{
		ID3D11ShaderResourceView* clearSRV = { NULL };

		// Set depth to disabled
		//ref->sys->dx->m_immediateContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);

		//ref->sys->dx->m_immediateContext->OMSetRenderTargets(1, &m_2DoverlayRTV, ref->sys->dx->m_depthStencilView);

#ifdef DEFERRED
		hr = m_2DdeferredContext->FinishCommandList(TRUE, &m_2DcommandList);
		
		if (m_2DcommandList)
		{
			//ref->sys->dx->m_immediateContext->OMSetRenderTargets(1, &m_2DoverlayRTV, ref->sys->dx->m_depthStencilView);

			// Execute all pending commands to draw to the overlay render target
			ref->sys->dx->m_immediateContext->ExecuteCommandList(m_2DcommandList, TRUE);

			SAFE_RELEASE(m_2DcommandList);
		}
#endif

/*		EndD2DDrawing();
		ref->sys->dx->subsystem2D->m_d2dRenderTarget->BeginDraw();
		ref->sys->dx->m_d2dContext->DrawImage(ref->sys->dx->m_d2dCommandList);
		hr = ref->sys->dx->subsystem2D->m_d2dRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			LOG(error) << "m_d2dRenderTarget->EndDraw() returned D2DERR_RECREATE_TARGET";
		}
*/
		//ref->sys->dx->m_immediateContext->PSSetShaderResources(0, 1, &clearSRV);

		// Set the back buffer as the current render target
		//ref->sys->dx->m_immediateContext->OMSetRenderTargets(1, &ref->sys->dx->m_backBufferRTV, ref->sys->dx->m_depthStencilView);

		// Calculate the width of the overlay.
		int width = static_cast<int>(static_cast<float>(m_renderTargetWidth) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the left side of the overlay.
		int x = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowWidth) - width) / 2);

		// Calculate the height of the overlay.
		int height = static_cast<int>(static_cast<float>(m_renderTargetHeight) * ref->cvars->overlayScale->Float());

		// Calculate the screen coordinates of the top of the overlay.
		int y = ((msl::utilities::SafeInt<int>(ref->sys->dx->m_windowHeight) - height) / 2);

		//ref->sys->dx->subsystem2D->m_2DdeferredContext->PSSetShaderResources(0, 1, &clearSRV);

		// Render 2D overlay
		m_renderTargetQuad.Render(x, y, width, height, DirectX::Colors::White);

		// Render the overlay to the back buffer
		m_2DshaderTexture.Render(ref->sys->dx->m_immediateContext, m_renderTargetQuad.IndexCount(), DirectX::XMMatrixIdentity(), DirectX::XMMatrixIdentity(), m_2DorthographicMatrix, m_2DshaderResourceView);

		// Clear the PS binding
		//ref->sys->dx->subsystem2D->m_2DdeferredContext->PSSetShaderResources(0, 1, &clearSRV);
		//ref->sys->dx->m_immediateContext->PSSetShaderResources(0, 1, &clearSRV);

		// Set the overlay RTV as the current render target
		//ref->sys->dx->subsystem2D->m_2DdeferredContext->OMSetRenderTargets(1, &m_2DoverlayRTV, ref->sys->dx->m_depthStencilView);
	}

#ifdef _DEBUG
	DumpD3DDebugMessagesToLog();
#endif
}

void dx11::Subsystem2D::FadeScreen()
{
	/*ActivateD2DDrawing();

	ref->sys->dx->m_d2dContext->FillRectangle(
		D2D1::RectF(
			0,
			0,
			m_renderTargetWidth,
			m_renderTargetHeight),
		fadeColor);*/
}

void dx11::Subsystem2D::Shutdown()
{
	LOG_FUNC();

	LOG(info) << "Shutting down.";

	EndD2DDrawing();

	m_2DshaderVertexColor.Shutdown();

	m_2DshaderTexture.Shutdown();

	m_generalPurposeQuad.Shutdown();

	m_renderTargetQuad.Shutdown();

	SAFE_RELEASE(colorYellowGreen);

	SAFE_RELEASE(colorGray);

	SAFE_RELEASE(colorBlack);

	SAFE_RELEASE(fadeColor);

	SAFE_RELEASE(m_alphaBlendState);

	SAFE_RELEASE(m_d2dRenderTarget);

	SAFE_RELEASE(m_dxgiSurface);

	SAFE_RELEASE(m_depthDisabledStencilState);

#ifdef DEFERRED
	SAFE_RELEASE(m_2DdeferredContext);
#endif

	SAFE_RELEASE(m_2DshaderResourceView);

	SAFE_RELEASE(m_2DoverlayRTV);

	SAFE_RELEASE(m_2DrenderTargetTexture);

	LOG(info) << "Shutdown complete.";
}
