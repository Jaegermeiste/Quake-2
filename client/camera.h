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
================================
camera.h
Psychospaz's 3rd Person Chasecam
Enhanced by NeVo
================================
*/

#ifndef __CAMERA_H__	// Protect camera.h
#define __CAMERA_H__

#pragma once
#include <cglm/cglm.h>

// Cameras
typedef enum camera_e {
	QCAMERA0,				// First Person Camera
	QCAMERA1,				// Third Person Chase Camera
	MAX_CAMERAS				// Maximum number of cameras
} camera_t;

typedef enum camtype_e {
	CAMTYPE_NONE,			// No type (unsupported)
	CAMTYPE_FIRSTPERSON,	// First person camera (through eyes)
	CAMTYPE_THIRDPERSON	// Chase camera (from behind)
} camtype_t;

typedef enum camzoom_e {
	CAMZOOM_NONE,			// No zoom
	CAMZOOM_AUTO_IN,		// Auto zoom out
	CAMZOOM_AUTO_OUT,		// Auto zoom in
	CAMZOOM_MANUAL_IN,		// Manual zoom in
	CAMZOOM_MANUAL_OUT		// Manual zoom out
} camzoom_t;

// Camera parameter list
typedef enum camopt_e {
	CAM_NONE,				// Unsupported
	CAM_TYPE,				// Camera type
	CAM_ZOOM,				// Zoom mode
	CAMMODE_DAMPED,			// Spring damped motion
	CAMMODE_SMART,			// Collision avoidance
	CAMMODE_CLIP,			// Collision detection
	CAMMODE_ALPHA,			// Viewer model alpha blending
	CAMMODE_ZOOM,			// Camera can be zoomed
	CAMMODE_MAGNIFY,		// Camera can be magnified
	CAMMODE_KICK			// Camera is affected by weapon recoil
} camopt_t;

// Our interface
typedef struct camdevice_s
{
	void      (*Activate)		    (camera_t cam);
	void      (*ActivateLast)	    (void);
	void      (*CreateCam)		    (camera_t cam, camtype_t type);
	camera_t  (*GetCurrentCam)	    (void);
	void*     (*GetOption)		    (camera_t cam, int mode);
	void      (*SetOption)		    (camera_t cam, camopt_t option, void* value);
	void      (*ShutdownCamera)	    (camera_t cam);
	void      (*ShutdownCameras)	(void);			// Shut down cameras
	void      (*Think)			    (qboolean updateextras);

	// These functions are provided due to common use
	void      (*ZoomIn)			    (void);			// Zoom in active camera
	void      (*ZoomOut)		    (void);			// Zoom out active camera
	void      (*ToEyeCam)		    (void);			// Switch to first person camera
	void      (*ToChaseCam)		    (void);			// Switch to chase camera
} camdevice_t;

// Device Properties
typedef struct camdevstate_s {
	int	DefinedCameras[MAX_CAMERAS];	// Array of defined cameras
	camera_t	ActiveCamera;					// Currently active camera
	camera_t	LastCamera;						// Last Active Camera
	float		viewermodelalpha;				// Player model transparency
	double		maxvertangle;					// Maximum vertical angle
	camzoom_t	zoommode;						// Current zoom mode
	int	locked;							// User can't adjust the camera
	int	Initialized;					// System has been initialized
	float		fov;							// Current FOV
	unsigned int oldtime;						// Old camera time
} camdevstate_t;

// Camera Properties
typedef struct camstate_s {

	// Camera Information
	camera_t	name;			// This camera's identifier
	camtype_t	type;			// Camera type
	qboolean	damped;			// Spring damped motion
	qboolean	smart;			// Collision avoidance
	qboolean	clip;			// Collision detection
	qboolean	alpha;			// Viewer model alpha blending
	qboolean	zoomable;		// Camera is zoomable
	qboolean	magnifiable;	// Camera can be magnified
	qboolean	kickable;		// Camera can be kicked

	// World Camera Position
	vec3_t		origin;			// This camera's current origin (where it is)

	// Camera Target Positions
	vec3_t		targetOrigin;	// This camera's destination origin (where it wants to be)
	vec3_t		oldTargetOrigin;// Last target

	// Origin to look at
	vec3_t		lookAtTarget;			// Origin to look at

	// Trailing Distance Parameters
	float		mindist;		// Minimum distance from vieworg
	float		normdist;		// Normal distance from vieworg
	float		maxdist;		// Maximum distance from vieworg
	float		currentdist;	// Current distance from vieworg

	// Motion Damping
	float		tether;			// Tether distance

	// Collision Detection and Avoidance
	float		radius;			// This camera's virtual radius

	// Model Transparency
	float		maxalphadist;	// Maximum distance to calculate alpha

	// Magnification
	float		magnification;
	float		maxmagnification;
	float		minmagnification;
	float		normmagnification;

	// Field of View
	float		fov;
} camstate_t;

#define MASK_CAMERA		(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER|CONTENTS_PLAYERCLIP|CONTENTS_TRANSLUCENT|CONTENTS_DETAIL)
#define	AUTOZOOM_UPF	5
#define MANUALZOOM_UPF	3
#define VELOCITY_DAMP	0.65
#define TELEPORT_DIST	256*8
#define MAX_FOV			179
#define MIN_FOV			1
#define CAMLERP_FRAC	0.05
#define CAM_PIXELLERP	1
#define SPRING_CONST	2.5f //4.5f //1.380658 // E^23
#define DAMP_CONST		0.5f //3.5f
#define QX				0
#define QY				1
#define QZ				2

////////////////////////////////////////
// Exported Camera Data and Functions //
////////////////////////////////////////

// Camera Device
extern	camdevice_t	camdevice;

// Cvars
extern	cvar_t* cl_thirdPerson;
extern	cvar_t* cl_thirdPersonAngle;
extern	cvar_t* cl_thirdPersonDist;
extern	cvar_t* cl_thirdPersonDamp;

// Camera System Functions
void Cam_InitSystem(void);
void Cam_Shutdown(void);

// Other Camera Related functions
void Cam_UpdateEntity(entity_t* ent);

// Console Commands (affect the current camera)
void Cam_Reset_f(void);
void Cam_Toggle_f(void);
void Cam_ZoomIn_f(void);
void Cam_ZoomOut_f(void);
//====================================//

////////////////////////////
// Model and Skin Bugfixes//
////////////////////////////
extern	cvar_t* gender;
extern	cvar_t* model;
extern	cvar_t* skin;
extern  cvar_t* fov;
//========================//

#endif	// __CAMERA_H__#pragma once
