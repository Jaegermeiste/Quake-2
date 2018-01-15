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

inline void	SHIM_R_BeginRegistration(char *map)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->BeginRegistration();

		if (dx11::ref->model != nullptr)
		{
			std::string mapName(map);

			dx11::ref->model->LoadMap(mapName);
		}
	}
}

inline model_s* SHIM_R_RegisterModel(char *name)
{
	model_s *model = nullptr;

	if ((dx11::ref != nullptr) && (dx11::ref->model != nullptr))
	{
		std::string modelName(name);

		model = dx11::ref->model->LoadModel(modelName).get();
	}

	return model;
}

inline struct image_s	*SHIM_R_RegisterSkin(char *name)
{
	image_s *image = nullptr;

	if ((dx11::ref != nullptr) && (dx11::ref->img != nullptr))
	{
		image = dx11::ref->img->Load(name, it_skin).get();
	}

	return image;
}

inline image_t	*SHIM_R_RegisterPic(char *name)
{
	image_s *image = nullptr;

	if ((dx11::ref != nullptr) && (dx11::ref->img != nullptr))
	{
		image = dx11::ref->img->Load(name, it_pic).get();
	}

	return image;
}

inline void SHIM_R_SetSky(char *name, float rotate, vec3_t axis)
{
	if ((dx11::ref != nullptr) && (dx11::ref->img != nullptr))
	{
		dx11::ref->img->Load(name, it_sky);
	}
}

inline void	SHIM_R_EndRegistration(void)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->EndRegistration();
	}
}

inline void	SHIM_R_RenderFrame(refdef_t *fd)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->RenderFrame(fd);
	}
}

inline void	SHIM_Draw_GetPicSize(int *w, int *h, char *name)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		unsigned int width = msl::utilities::SafeInt<unsigned int>(*w);
		unsigned int height = msl::utilities::SafeInt<unsigned int>(*h);
		dx11::ref->draw->GetPicSize(width, height, name);
		*w = msl::utilities::SafeInt<int>(width);
		*h = msl::utilities::SafeInt<int>(height);
	}
}

inline void	SHIM_Draw_Pic(int x, int y, char *name)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->Pic(x, y, name);
	}
}

inline void	SHIM_Draw_StretchPic(int x, int y, int w, int h, char *name)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->StretchPic(msl::utilities::SafeInt<unsigned int>(x),
								msl::utilities::SafeInt<unsigned int>(y), 
								msl::utilities::SafeInt<unsigned int>(w), 
								msl::utilities::SafeInt<unsigned int>(h), 
								name);
	}
}

inline void	SHIM_Draw_Char(int x, int y, int c)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->Char(msl::utilities::SafeInt<unsigned int>(x), msl::utilities::SafeInt<unsigned int>(y), c);
	}
}

inline void	SHIM_Draw_TileClear(int x, int y, int w, int h, char *name)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->TileClear(msl::utilities::SafeInt<unsigned int>(x),
							msl::utilities::SafeInt<unsigned int>(y),
							msl::utilities::SafeInt<unsigned int>(w),
							msl::utilities::SafeInt<unsigned int>(h),
							name);
	}
}

inline void	SHIM_Draw_Fill(int x, int y, int w, int h, int c)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->Fill(msl::utilities::SafeInt<unsigned int>(x),
						msl::utilities::SafeInt<unsigned int>(y),
						msl::utilities::SafeInt<unsigned int>(w),
						msl::utilities::SafeInt<unsigned int>(h),
						c);
	}
}

inline void	SHIM_Draw_FadeScreen(void)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->FadeScreen();
	}
}

inline void	SHIM_Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
	if ((dx11::ref != nullptr) && (dx11::ref->draw != nullptr))
	{
		dx11::ref->draw->StretchRaw(msl::utilities::SafeInt<unsigned int>(x),
			msl::utilities::SafeInt<unsigned int>(y),
			msl::utilities::SafeInt<unsigned int>(w),
			msl::utilities::SafeInt<unsigned int>(h),
			msl::utilities::SafeInt<unsigned int>(cols),
			msl::utilities::SafeInt<unsigned int>(rows),
			data);
	}
}

inline qboolean SHIM_R_Init	(void *hinstance, void *wndproc)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		HINSTANCE hInstance = static_cast<HINSTANCE>(hinstance);
		WNDPROC wndProc = static_cast<WNDPROC>(wndproc);
		bool retVal = dx11::ref->sys->Initialize(hInstance, wndProc);
		if (retVal == true)
		{
			return true;
		}
	}
	return false;
}

inline void SHIM_R_Shutdown()
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->Shutdown();
	}
}

inline void SHIM_R_SetPalette(const unsigned char *palette)
{

}

inline void	SHIM_R_BeginFrame(float camera_separation)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->BeginFrame();
	}
}

inline void SHIM_R_EndFrame(void)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		dx11::ref->sys->EndFrame();
	}
}

inline void SHIM_R_AppActivate(qboolean active)
{
	if ((dx11::ref != nullptr) && (dx11::ref->sys != nullptr))
	{
		if (active == true)
		{
			dx11::ref->sys->AppActivate(true);
		}
		else
		{
			dx11::ref->sys->AppActivate(false);
		}
	}
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t GetRefAPI(refimport_t rimp)
{
	refexport_t	re;

	if (dx11::ref == nullptr)
	{
		dx11::ref = std::make_unique<dx11::Ref>(rimp);
	}

	re.api_version			= API_VERSION;

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

	return re;
}