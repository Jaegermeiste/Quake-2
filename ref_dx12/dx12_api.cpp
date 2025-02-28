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

inline void	SHIM_R_BeginRegistration(char* map)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->media->BeginRegistration(ToWideString(map));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline model_s* SHIM_R_RegisterModel(char* name)
{
	model_s* model = nullptr;

	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->model != nullptr) && (dx12::ref->sys != nullptr))
		{
			std::wstring modelName(ToWideString(name));

			// In the client, the addresses of the returned values are compared in equality tests (etc.), so can't return nullptr. Still an opaque type.
			auto resource = dx12::ref->media->model->Load(modelName);

			if (resource) {
				model = reinterpret_cast<struct model_s*>(dx12::ref->res->GetResourceHandleQuake2(resource->GetHandle()));
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return model;
}

inline struct image_s* SHIM_R_RegisterSkin(char* name)
{
	image_s* image = nullptr;

	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->img != nullptr) && (dx12::ref->sys != nullptr))
		{
			auto resource = dx12::ref->media->img->Load(ToWideString(name), it_skin);

			if (resource) {
				image = reinterpret_cast<struct image_s*>(dx12::ref->res->GetResourceHandleQuake2(resource->GetHandle()));
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return image;
}

inline image_s* SHIM_R_RegisterPic(char* name)
{
	image_s* image = nullptr;

	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->img != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->media->img->Load(ToWideString(name), it_pic);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return image;
}

inline void SHIM_R_SetSky(char* name, float rotate, vec3_t axis)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->img != nullptr) && (dx12::ref->sys != nullptr) && (dx12::ref->sys->dx != nullptr) && (dx12::ref->sys->dx->subsystem3D != nullptr))
		{
			dx12::ref->media->img->LoadSky(ToWideString(name));

			dx12::ref->sys->dx->subsystem3D->SetSkyboxRotation(rotate, axis);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_R_EndRegistration(void)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr))
		{
			dx12::ref->media->EndRegistration();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_R_RenderFrame(refdef_t* fd)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr) && (dx12::ref->sys->dx != nullptr))
		{
			dx12::ref->sys->dx->RenderFrame(fd);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_GetPicSize(int* w, int* h, char* name)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr) && (dx12::ref->sys != nullptr))
		{
			unsigned int	width = 0,
				height = 0;
			dx12::ref->draw->GetPicSize(width, height, ToWideString(name));
			*w = msl::utilities::SafeInt<int>(width);
			*h = msl::utilities::SafeInt<int>(height);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_Pic(int x, int y, char* name)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->draw->Pic(x, y, ToWideString(name));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_StretchPic(int x, int y, int w, int h, char* name)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->draw->StretchPic(msl::utilities::SafeInt<int>(x),
				msl::utilities::SafeInt<int>(y),
				msl::utilities::SafeInt<int>(w),
				msl::utilities::SafeInt<int>(h),
				ToWideString(name));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_Char(int x, int y, int c)
{
	try
	{
		if ((c < 0) || (y <= -SMALL_CHAR_SIZE) || ((c & 127) == 32))
		{
			// Invalid, offscreen, or space
			return;
		}

		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr))
		{
			dx12::ref->draw->Char(x, y, msl::utilities::SafeInt<unsigned char>(c));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_TileClear(int x, int y, int w, int h, char* name)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->draw->TileClear(msl::utilities::SafeInt<int>(x),
				msl::utilities::SafeInt<int>(y),
				msl::utilities::SafeInt<int>(w),
				msl::utilities::SafeInt<int>(h),
				ToWideString(name));
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_Fill(int x, int y, int w, int h, int c)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr))
		{
			dx12::ref->draw->Fill(msl::utilities::SafeInt<int>(x),
				msl::utilities::SafeInt<int>(y),
				msl::utilities::SafeInt<int>(w),
				msl::utilities::SafeInt<int>(h),
				c);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_FadeScreen(void)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr))
		{
			dx12::ref->draw->FadeScreen();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte* data)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->draw != nullptr))
		{
			dx12::ref->draw->StretchRaw(msl::utilities::SafeInt<int>(x),
				msl::utilities::SafeInt<int>(y),
				msl::utilities::SafeInt<int>(w),
				msl::utilities::SafeInt<int>(h),
				msl::utilities::SafeInt<unsigned int>(cols),
				msl::utilities::SafeInt<unsigned int>(rows),
				data);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline qboolean SHIM_R_Init(void* hinstance, void* wndproc)
{
	try
	{
		if ((dx12::Initialize()) && (dx12::ref != nullptr) && (dx12::ref->sys != nullptr))
		{
			HINSTANCE hInstance = static_cast<HINSTANCE>(hinstance);
			WNDPROC wndProc = static_cast<WNDPROC>(wndproc);
			bool retVal = dx12::ref->sys->Initialize(hInstance, wndProc);
			if (retVal == true)
			{
				retVal = dx12::ref->media->Initialize();

				if (retVal == true)
				{
					return true;
				}
			}
		}
		return false;
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

inline void SHIM_R_Shutdown()
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr))
		{
			dx12::ref->media->Shutdown();
		}

		if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr))
		{
			dx12::ref->sys->Shutdown();
		}

		dx12::Shutdown();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void SHIM_R_SetPalette(const unsigned char* palette)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->media != nullptr) && (dx12::ref->media->img != nullptr))
		{
			dx12::ref->media->img->SetRawPalette(palette);
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void	SHIM_R_BeginFrame(float camera_separation)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr) && (dx12::ref->sys->dx != nullptr))
		{
			camera_separation = 0.0f; // Silence "C4100 'camera_separation': unreferenced formal parameter" since we are deliberately and completely discarding this parameter

			dx12::ref->sys->dx->BeginFrame();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void SHIM_R_EndFrame(void)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr) && (dx12::ref->sys->dx != nullptr))
		{
			dx12::ref->sys->dx->EndFrame();
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

inline void SHIM_R_AppActivate(qboolean active)
{
	try
	{
		if ((dx12::ref != nullptr) && (dx12::ref->sys != nullptr))
		{
			if (active == true)
			{
				dx12::ref->sys->AppActivate(true);
			}
			else
			{
				dx12::ref->sys->AppActivate(false);
			}
		}
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
extern "C" __declspec(dllexport) refexport_t GetRefAPI(refimport_t rimp)
{
	LOG_FUNC();

	refexport_t	re = {};

	try
	{
		if (dx12::log == nullptr)
		{
			dx12::log = std::make_unique<dx12::Log>();
		}

		if (dx12::ref == nullptr)
		{
			dx12::ref = std::make_unique<dx12::Ref>();
			dx12::ref->Init(rimp);
		}

		re.api_version = API_VERSION;

		re.BeginRegistration	= SHIM_R_BeginRegistration;
		re.RegisterModel		= SHIM_R_RegisterModel;
		re.RegisterSkin			= SHIM_R_RegisterSkin;
		re.RegisterPic			= SHIM_R_RegisterPic;
		re.SetSky				= SHIM_R_SetSky;
		re.EndRegistration		= SHIM_R_EndRegistration;

		re.RenderFrame			= SHIM_R_RenderFrame;

		re.DrawGetPicSize		= SHIM_Draw_GetPicSize;
		re.DrawPic				= SHIM_Draw_Pic;
		re.DrawStretchPic		= SHIM_Draw_StretchPic;
		re.DrawChar				= SHIM_Draw_Char;
		re.DrawTileClear		= SHIM_Draw_TileClear;
		re.DrawFill				= SHIM_Draw_Fill;
		re.DrawFadeScreen		= SHIM_Draw_FadeScreen;
		re.DrawStretchRaw		= SHIM_Draw_StretchRaw;

		re.Init					= SHIM_R_Init;
		re.Shutdown				= SHIM_R_Shutdown;

		re.CinematicSetPalette	= SHIM_R_SetPalette;
		re.BeginFrame			= SHIM_R_BeginFrame;
		re.EndFrame				= SHIM_R_EndFrame;

		re.AppActivate			= SHIM_R_AppActivate;

		Swap_Init();
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return re;
}