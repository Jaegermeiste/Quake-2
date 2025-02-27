/*
cl_camera.c

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
Psychospaz's Chase Camera Code
NeVo - exported the entire original chasecam into this file on 01 Sept, 2002
NeVo - performed massive updates to this code on 28, 29, and 30 Nov, 2002 (yes, thanksgiving)
NeVo - got rid of animations on 31 Nov, 2002
NeVo - more massive updates on 02 and 03 Dec, 2002
NeVo - brought animations back on 03 Dec, 2002
NeVo - performed gigantic update and bugfix from 03 to 07 Dec, 2002.
NeVo - brought in loading of camera definitions thru .cdf files 07 Dec, 2002
NeVo - disabled .cdf files 08 Dec, 2002
NeVo - dampened camera motion 11 Dec, 2002
NeVo - dumped animations again 27 Dec, 2002
NeVo - massive interface overhaul 27, 28 Dec, 2002
NeVo - began final phase of project 26 Jan, 2003
NeVo - almost finished code 12 Feb, 2003
NeVo - plugins system implemented 13 Feb, 2003
NeVo - started reimplementing animations 15 Feb, 2003
NeVo - animations implemented as plugins 15 Feb, 2003
NeVo - lookat system implemented 15 Feb, 2003
NeVo - back to this code 01 April, 2003
NeVo - implemented camera prediction 01 April, 2003
NeVo - hopefully fixed spastic camera bug 01 April, 2003
NeVo - extended camera prediction 01 April, 2003
NeVo - removed extended camera prediction 02 April, 2003
NeVo - definitely fixed spastic camera 02 April, 2003

A note on multiple cameras: Extra cameras could be used to switch fixed
views (Alone in the Dark style), used for special animations,
camera angles in demos, etc. To use multiple cameras the extra
camera(s) properties must be defined. The default camera is QCAMERA0.

This code is based off of Psychospaz's original chasecam tutorial and the
Quake2Max 0.42 source code. NeVo does not claim credit for code
written by Psychospaz. This is merely an advanced implementation of the
original under the GPL.

Camera Version 5.3 - 28 Feb, 2003 (NeVo)
*/

#include <stringzilla/stringzilla.h>
#include "../qcommon/qcommon.h"
#include "client.h"

// Default cameras
camdevice_t		camdevice;				// Main Camera Device
camdevstate_t	camdevstate;			// Device State
camstate_t* cameras[MAX_CAMERAS];	// Array of cameras

// Cvars
cvar_t* cl_thirdPerson;
cvar_t* cl_thirdPersonAngle;
cvar_t* cl_thirdPersonDist;
cvar_t* cl_thirdPersonDamp;

/************************
** Function Prototypes **
************************/
float absval(float a);
void CamSystem_Activate(camera_t cam);
void CamSystem_ActivateLast(void);
void CamSystem_CreateCam(camera_t cam, camtype_t type);
void* CamSystem_GetOption(camera_t cam, camopt_t option);
camera_t CamSystem_ReturnCam(void);
void CamSystem_SetOption(camera_t cam, camopt_t option, void* value);
void CamSystem_ShutdownCam(camera_t cam);
void CamSystem_ShutdownCams(void);
void CamSystem_Think(qboolean updateextras);
void CamSystem_ToChase(void);
void CamSystem_ToEyes(void);
void CamSystem_ZoomIn(void);
void CamSystem_ZoomOut(void);
void CamEngine_AddEntAlpha(entity_t* ent);
void CamEngine_CalcAlpha(camera_t cam);
void CamEngine_CalcLookat(camera_t cam, player_state_t* ps, player_state_t* ops);
void CamEngine_Clip(camera_t cam);
void CamEngine_DirectView(camera_t cam);
void CamEngine_FOV(camera_t cam);
void CamEngine_InitPosition(vec3_t myViewOrg);
void CamEngine_Kick(camera_t cam);
void CamEngine_ParseState(camera_t cam);
void CamEngine_SmartTarget(camera_t cam);
void CamEngine_Damp(camera_t cam);
void CamEngine_Think(camera_t cam);
trace_t CamEngine_Trace(vec3_t start, vec3_t end, float size, int contentmask);
void CamEngine_UpdatePrimary(camera_t cam);
void CamEngine_UpdateTarget(camera_t cam);
void CamEngine_Zoom(camera_t cam);
void Cam_InitSystem(void);
void Cam_Shutdown(void);
void Cam_UpdateEntity(entity_t* ent);
void Cam_Reset_f(void);
void Cam_Toggle_f(void);
void Cam_ZoomIn_f(void);
void Cam_ZoomOut_f(void);
extern void CL_AddViewWeapon(player_state_t* ps, player_state_t* ops);
extern void vectoangles2(vec3_t value1, vec3_t angles);

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

/*
||||||||||||||||||||||||||||||||||||
			Camera System
||||||||||||||||||||||||||||||||||||
This section contains the camera system functions.
*/

/*
==================
CamSystem_Activate

Swtich cameras
==================
*/
void CamSystem_Activate(camera_t cam)
{
	// If the camera is defined, switch to it
	if (camdevstate.DefinedCameras[cam])
	{
		camdevstate.LastCamera = camdevice.GetCurrentCam();
		camdevstate.ActiveCamera = cam;

		if (cam == QCAMERA0)
		{
			cl_thirdPerson->value = 0;
		}
		else if (cam == QCAMERA1)
		{
			cl_thirdPerson->value = 1;
		}

		Com_Printf("Camera %i\n", cam);
	}
}

/*
======================
CamSystem_ActivateLast

Swtich cameras
======================
*/
void CamSystem_ActivateLast(void)
{
	// If the camera is defined, switch to it
	if (camdevstate.DefinedCameras[camdevstate.LastCamera])
	{
		int oldcam = camdevice.GetCurrentCam();
		camdevstate.ActiveCamera = camdevstate.LastCamera;
		camdevstate.LastCamera = oldcam;
	}
}

/*
===================
CamSystem_CreateCam

Create a camera
===================
*/
void CamSystem_CreateCam(camera_t cam, camtype_t type)
{
	if (camdevstate.DefinedCameras[cam])
	{
		return;
	}
	else
	{
		vec3_t viewOrg;
		memset(viewOrg, 0, sizeof(vec3_t));

		CamEngine_InitPosition(viewOrg);

		// Allocate some ram
		cameras[cam] = Z_Malloc(sizeof(camstate_t));

		if (cameras[cam] != NULL)
		{
			// Camera Information
			cameras[cam]->name = cam;
			cameras[cam]->type = type;
			cameras[cam]->damped = false;
			cameras[cam]->smart = false;
			cameras[cam]->clip = false;
			cameras[cam]->alpha = false;
			cameras[cam]->zoomable = false;
			cameras[cam]->magnifiable = false;

			switch (type) {
			case CAMTYPE_FIRSTPERSON:
				cameras[cam]->currentdist = 0.0f;
				cameras[cam]->maxdist = 0.0f;
				cameras[cam]->mindist = 0.0f;
				cameras[cam]->normdist = 0.0f;
				cameras[cam]->fov = fov->value;
				cameras[cam]->magnification = 1.0f;
				cameras[cam]->maxmagnification = 1.0f;
				cameras[cam]->minmagnification = 1.0f;
				cameras[cam]->normmagnification = 1.0f;
				cameras[cam]->maxalphadist = 0.0f;
				cameras[cam]->radius = 1.0f;
				cameras[cam]->tether = 0.00001f;
				cameras[cam]->zoomable = false;
				cameras[cam]->kickable = true;
				VectorCopy(viewOrg, cameras[cam]->origin);
				VectorCopy(viewOrg, cameras[cam]->targetOrigin);
				VectorCopy(viewOrg, cameras[cam]->oldTargetOrigin);
				break;
			case CAMTYPE_THIRDPERSON:
				cameras[cam]->alpha = true;
				cameras[cam]->clip = true;
				cameras[cam]->currentdist = cl_thirdPersonDist->value;
				cameras[cam]->mindist = 3.0f;
				cameras[cam]->maxdist = cl_thirdPersonDist->value + cameras[cam]->mindist;
				cameras[cam]->normdist = (cl_thirdPersonDist->value * 0.5f) + cameras[cam]->mindist;
				cameras[cam]->damped = true;
				cameras[cam]->fov = fov->value;
				cameras[cam]->magnification = 1.0f;
				cameras[cam]->maxmagnification = 1.0f;
				cameras[cam]->minmagnification = 1.0f;
				cameras[cam]->magnifiable = false;
				cameras[cam]->normmagnification = 1.0f;
				cameras[cam]->maxalphadist = 16.5f;
				cameras[cam]->radius = 4.0f;
				cameras[cam]->smart = true;
				cameras[cam]->tether = 3.8f;		// No more than 4.0, or prediction gets choppy
				cameras[cam]->zoomable = true;
				cameras[cam]->kickable = false;
				VectorCopy(viewOrg, cameras[cam]->origin);
				VectorCopy(viewOrg, cameras[cam]->targetOrigin);
				VectorCopy(viewOrg, cameras[cam]->oldTargetOrigin);
			default:
				break;
			}

			VectorCopy(cl.refdef.vieworg, cameras[cam]->origin);
		}
	}

	camdevstate.DefinedCameras[cam] = true;
	camdevice.Activate(cam);
}

/*
===================
CamSystem_GetOption

Return value of parameter
===================
*/
void* CamSystem_GetOption(camera_t cam, camopt_t option)
{
	switch (option) {
	case CAM_TYPE:					// Camera type
		return (void*)cameras[cam]->type;
	case CAM_ZOOM:					// Zoom mode
		return (void*)camdevstate.zoommode;
	case CAMMODE_DAMPED:			// Spring damped motion
		return (void*)cameras[cam]->damped;
	case CAMMODE_SMART:			// Collision avoidance
		return (void*)cameras[cam]->smart;
	case CAMMODE_CLIP:				// Collision detection
		return (void*)cameras[cam]->clip;
	case CAMMODE_ALPHA:			// Viewer model alpha blending
		return (void*)cameras[cam]->alpha;
	case CAMMODE_ZOOM:				// Zoomable
		return (void*)cameras[cam]->zoomable;
	case CAMMODE_MAGNIFY:			// Magnifiable
		return (void*)cameras[cam]->magnifiable;
	case CAMMODE_KICK:				// Recoil, Pain
		return (void*)cameras[cam]->kickable;
	}
	return NULL;
}

/*
===================
CamSystem_ReturnCam

Return index of active camera
===================
*/
camera_t CamSystem_ReturnCam(void)
{
	return camdevstate.ActiveCamera;
}

/*
===================
CamSystem_SetOption

Set parameter to value
===================
*/
void CamSystem_SetOption(camera_t cam, camopt_t option, void* value)
{
	if (!value)
		return;

	switch (option) 
	{
	case CAM_TYPE:					// Camera type
		cameras[cam]->type = (camtype_t)value;
		break;
	case CAM_ZOOM:					// Zoom mode
		camdevstate.zoommode = (camzoom_t)value;
		break;
	case CAMMODE_DAMPED:			// Spring damped motion
		cameras[cam]->damped = (qboolean)value;
		break;
	case CAMMODE_SMART:			// Collision avoidance
		cameras[cam]->smart = (qboolean)value;
		break;
	case CAMMODE_CLIP:				// Collision detection
		cameras[cam]->clip = (qboolean)value;
		break;
	case CAMMODE_ALPHA:			// Viewer model alpha blending
		cameras[cam]->alpha = (qboolean)value;
		break;
	case CAMMODE_ZOOM:				// Zoomable
		cameras[cam]->zoomable = (qboolean)value;
		break;
	case CAMMODE_MAGNIFY:			// Magnifiable
		cameras[cam]->magnifiable = (qboolean)value;
		break;
	case CAMMODE_KICK:				// Recoil, Pain
		cameras[cam]->kickable = (qboolean)value;
		break;
	default:
		break;
	}
	return;
}

/*
=====================
CamSystem_ShutdownCam

Shutdown a camera
=====================
*/
void CamSystem_ShutdownCam(camera_t cam)
{
	// We can't shut down the default cameras manually
	if (!(cam > QCAMERA1))
		return;

	if(cam < MAX_CAMERAS)
	{
		Z_Free(cameras[cam]);
		cameras[cam] = NULL;
		camdevstate.DefinedCameras[cam] = false;
	}

	camdevice.Activate(QCAMERA0);
}

/*
======================
CamSystem_ShutdownCams

Shutdown all cameras
!! This should only ever be called at client shutdown !!
======================
*/
void CamSystem_ShutdownCams(void)
{
	int i = 0;

	for (i = 0; i < MAX_CAMERAS; i++)
	{
		if (cameras[i] != NULL)
		{
			Z_Free(cameras[i]);
			cameras[i] = NULL;
		}
		camdevstate.DefinedCameras[i] = false;
	}
}

/*
===============
CamSystem_Think

Process cameras
Set updateextras to true to parse all cameras
===============
*/
void CamSystem_Think(qboolean updateextras)
{
	camera_t cam = camdevice.GetCurrentCam();

	CamEngine_ParseState(cam);

	if (updateextras)
	{
		int i;
		// Update all cameras
		for (i = 0; i < MAX_CAMERAS; i++)
		{
			CamEngine_Think(i);
		}
	}
	else
	{
		// Always update the default cameras to prevent switching artifacts
		CamEngine_Think(QCAMERA0);
		CamEngine_Think(QCAMERA1);

		if ((cam != QCAMERA0) && (cam != QCAMERA1))
		{
			// Update the current camera
			CamEngine_Think(cam);
		}
	}
}

/*
NeVo
TODO: The behavior of these two functions is unpredicatable
when switching from an outside camera (like a spectator
or security cam)...
*/

/*
=================
CamSystem_ToChase

Switch to chase camera
=================
*/
void CamSystem_ToChase(void)
{
	int cam = camdevice.GetCurrentCam();

	cl_thirdPerson->value = 1;

	if (cam == QCAMERA1)	// If already chase, this doesn't apply
		return;

	camdevice.Activate(QCAMERA1);
	camdevice.SetOption(QCAMERA1, CAM_ZOOM, (void*)CAMZOOM_AUTO_OUT);
}

/*
================
CamSystem_ToEyes

Switch to first person view
================
*/
void CamSystem_ToEyes(void)
{
	int cam = camdevice.GetCurrentCam();

	cl_thirdPerson->value = 0;

	if (cam == QCAMERA0)	// If first person, this doesn't apply
		return;

	camdevice.SetOption(cam, CAM_ZOOM, (void*)CAMZOOM_AUTO_IN);	// NeVo - another bug fix
}

/*
================
CamSystem_ZoomIn

Zoom camera in
================
*/
void CamSystem_ZoomIn(void)
{
	int cam = camdevice.GetCurrentCam();

	if (cam == QCAMERA0)	// If first person, this doesn't apply
	{
		cl_thirdPerson->value = 0;
		return;
	}

	camdevice.SetOption(cam, CAM_ZOOM, (void*)CAMZOOM_MANUAL_IN);
}

/*
=================
CamSystem_ZoomOut

Zoom camera out
=================
*/
void CamSystem_ZoomOut(void)
{
	int cam = camdevice.GetCurrentCam();

	if (cam == QCAMERA0)
	{
		cl_thirdPerson->value = 1;

		camdevice.Activate(QCAMERA1);
		camdevice.SetOption(QCAMERA1, CAM_ZOOM, (void*)CAMZOOM_MANUAL_OUT);
		return;
	}

	camdevice.SetOption(cam, CAM_ZOOM, (void*)CAMZOOM_MANUAL_OUT);
}

/*
|||||||||||||||||||||||||||||||||||||
			Camera Engine
|||||||||||||||||||||||||||||||||||||
This section contains the camera engine functions.
*/

/*
=====================
CamEngine_AddEntAlpha
=====================
*/
void CamEngine_AddEntAlpha(entity_t* ent)
{
	if (ent->alpha == 0)
		ent->alpha = camdevstate.viewermodelalpha;
	else
		ent->alpha *= camdevstate.viewermodelalpha;	// Set entity alpha

	if (ent->alpha < 1.0)						// If translucent, notify refresh
		ent->flags |= RF_TRANSLUCENT;
}

/*
===================
CamEngine_CalcAlpha

Calculate player model alpha
===================
*/
void CamEngine_CalcAlpha(camera_t cam)
{
	if (!camdevice.GetOption(cam, CAMMODE_ALPHA))
		return;

	vec3_t	displacement;

	// Calculate displacement from camera to player head
	VectorSubtract(cl.refdef.vieworg, cameras[cam]->origin, displacement);
	float actualDist = VectorLength(displacement);

	// Set alpha to one by default
	camdevstate.viewermodelalpha = 1.0;

	// Prevent division by zero
	if (cameras[cam]->maxalphadist == 0.0)
		cameras[cam]->maxalphadist = 0.1;

	// Make alpha from being zoomed too close
	if (actualDist <= cameras[cam]->maxalphadist)
		camdevstate.viewermodelalpha = actualDist / cameras[cam]->maxalphadist;

	// Clamp
	if (camdevstate.viewermodelalpha < 0.0)
		camdevstate.viewermodelalpha = 0.0;
	else if (camdevstate.viewermodelalpha > 1.0)	// Never more solid than solid
		camdevstate.viewermodelalpha = 1.0;
}

/*
====================
CamEngine_CalcLookat

Cipher View Angles
====================
*/
void get_direction_vectors_from_mat(mat4 rotation, vec3 forward, vec3 right, vec3 up) {
	static vec3 localRight =   { 1.0f, 0.0f, 0.0f }; // Default OpenGL right direction (X)
	static vec3 localUp =      { 0.0f, 1.0f, 0.0f }; // Default OpenGL up direction (Y)
	static vec3 localForward = { 0.0f, 0.0f, -1.0f }; // Default OpenGL forward direction (-Z)

	// Extract basis vectors from the matrix
	glm_mat4_mulv3(rotation, localRight, 0.0f, right);
	glm_mat4_mulv3(rotation, localUp, 0.0f, up);
	glm_mat4_mulv3(rotation, localForward, 0.0f, forward);
}

void CamEngine_CalcLookat(camera_t cam, player_state_t* ps, player_state_t* ops)
{
	int lerp = cl.lerpfrac;
	trace_t	tr;
	int i = 0;
	vec3_t traceEnd;
	vec3_t forward, right, up;

	// if not running a demo or on a locked frame, add the local angle movement
	if (cl.frame.playerstate.pmove.pm_type < PM_DEAD)
	{	// use predicted values
		for (i = 0; i < 3; i++)
			cl.refdef.viewangles[i] = cl.predicted_angles[i];
	}
	else
	{	// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.viewangles[i] = LerpAngle(ops->viewangles[i], ps->viewangles[i], lerp);
	}

	AngleVectors(cl.refdef.viewangles, &forward, &right, &up);

	// Figure out what we are looking at
	VectorMA(cl.refdef.vieworg, 512, forward, traceEnd);

	tr = CamEngine_Trace (cl.refdef.vieworg, traceEnd, 0.01f, MASK_CAMERA);
	VectorCopy(tr.endpos, cameras[cam]->lookAtTarget);
}

/*
==============
CamEngine_Clip

Clip camera
==============
*/
void CamEngine_Clip(camera_t cam)
{
	trace_t tr;

	if (!camdevice.GetOption(cam, CAMMODE_CLIP))
		return;

	// Clip the camera
	tr = CamEngine_Trace(cl.refdef.vieworg, cameras[cam]->origin, cameras[cam]->radius, MASK_CAMERA);

	if (tr.fraction != 1.0f)
	{
		VectorCopy(tr.endpos, cameras[cam]->origin);
	}
}

/*
=============
CamEngine_FOV

Do field of view adjustments
=============
*/
void CamEngine_FOV(camera_t cam)
{
	static float	lerp = 0.0f, old_fov = 90.0f;
	float			calc_fov = 90.0f;

	if (!camdevstate.Initialized)
		return;

	if (cameras[cam]->magnifiable)
		calc_fov = cameras[cam]->fov / cameras[cam]->magnification;
	else
		calc_fov = cameras[cam]->fov;

	if (!old_fov)
		old_fov = calc_fov;

	// interpolate field of view
	if ((calc_fov != old_fov) && (lerp <= 1.0))
	{
		//cl.refdef.fov_x = ops->fov + lerp * (ps->fov - ops->fov);
		camdevstate.fov = old_fov + lerp * (calc_fov - old_fov);
		cl.refdef.fov_x = camdevstate.fov;
		lerp += CAMLERP_FRAC;
	}
	else
	{
		cl.refdef.fov_x = camdevstate.fov = old_fov = calc_fov;
		lerp = 0.0;
	}

	// Make sure we don't break the system limits
	if (cl.refdef.fov_x > MAX_FOV)
		cl.refdef.fov_x = MAX_FOV;
	else if (cl.refdef.fov_x < MIN_FOV)
		cl.refdef.fov_x = MIN_FOV;
}

/*
======================
CamEngine_InitPosition

Return current vieworg position
NeVo - this exists to fix the camera init bug
======================
*/
void CamEngine_InitPosition(vec3_t myViewOrg)
{
	int			i = 0;
	float		lerp = 0.0f, backlerp = 0.0f;
	centity_t* ent = NULL;
	frame_t* oldframe = NULL;
	player_state_t* ps = NULL, * ops = NULL;

	// find the previous frame to interpolate from
	ps = &cl.frame.playerstate;
	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or invalid
	ops = &oldframe->playerstate;

	// see if the player entity was teleported this frame
	if (Q_fabs(ops->pmove.origin[0] - ps->pmove.origin[0]) > TELEPORT_DIST
		|| Q_fabs(ops->pmove.origin[1] - ps->pmove.origin[1]) > TELEPORT_DIST
		|| Q_fabs(ops->pmove.origin[2] - ps->pmove.origin[2]) > TELEPORT_DIST)
		ops = ps;		// don't interpolate

	ent = &cl_entities[cl.playernum + 1];
	lerp = cl.lerpfrac;

	// calculate the origin - modified by Psychospaz for 3rd person camera
	if ((cl_predict->value) && !(cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{	// use predicted values
		unsigned	delta;

		backlerp = 1.0 - lerp;
		for (i = 0; i < 3; i++)
		{
			myViewOrg[i] = cl.predicted_origin[i] + ops->viewoffset[i]
				+ cl.lerpfrac * (ps->viewoffset[i] - ops->viewoffset[i])
				- backlerp * cl.prediction_error[i];
		}

		// smooth out stair climbing
		delta = cls.realtime - cl.predicted_step_time;
		if (delta < 100)
			myViewOrg[2] -= cl.predicted_step * (100 - delta) * 0.01;
	}
	else
	{	// just use interpolated values
		for (i = 0; i < 3; i++)
			myViewOrg[i] = ops->pmove.origin[i] * 0.125 + ops->viewoffset[i]
			+ lerp * (ps->pmove.origin[i] * 0.125 + ps->viewoffset[i]
				- (ops->pmove.origin[i] * 0.125 + ops->viewoffset[i]));
	}
}

/*
==============
CamEngine_Kick

Kick the camera
==============
*/
void CamEngine_Kick(camera_t cam)
{
	int i = 0, lerp = cl.lerpfrac;
	player_state_t* ps = NULL, * ops = NULL;
	frame_t* oldframe = NULL;

	if (!cameras[cam]->kickable)
		return;

	// find the previous frame to interpolate from
	ps = &cl.frame.playerstate;
	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or invalid
	ops = &oldframe->playerstate;

	for (i = 0; i < 3; i++)
		cl.refdef.viewangles[i] *= LerpAngle(ops->kick_angles[i], ps->kick_angles[i], lerp);
}

/*
====================
CamEngine_ParseState

Cipher player state
====================
*/
void CamEngine_ParseState(camera_t cam)
{
	if (cl_thirdPerson->modified)
	{
		Cam_Toggle_f();
		cl_thirdPerson->modified = false;
	}
}

/*
=====================
CamEngine_SmartTarget

Collision Avoidance
=====================
*/
void CamEngine_SmartTarget(camera_t cam)
{
	vec3_t predictedNextTarget, predictedOffset, midpoint;
	trace_t tr;
	int	i = 0;

	if (!camdevice.GetOption(cam, CAMMODE_SMART))
		return;

	/*
	NeVo - added crude prediction for cameras in motion. This should smooth
	cameras going through doorways and stuff like that.
	*/
	if (!VectorCompare(cameras[cam]->targetOrigin, cameras[cam]->oldTargetOrigin))
	{
		// Roughly predict ahead once
		VectorSubtract(cameras[cam]->targetOrigin, cameras[cam]->oldTargetOrigin, predictedNextTarget);
		VectorAdd(predictedNextTarget, cameras[cam]->targetOrigin, predictedNextTarget);
		tr = CamEngine_Trace(cameras[cam]->targetOrigin, predictedNextTarget, cameras[cam]->radius, MASK_CAMERA);

		// Influence the targetOrigin based on our prediction
		for (i = 0; i < 3; i++)
			cameras[cam]->targetOrigin[i] = cameras[cam]->targetOrigin[i] + ((1.0 - tr.fraction) * (tr.endpos[i] - (cameras[cam]->targetOrigin[i])));

		/*// Roughly predict further ahead
		VectorSubtract (tr.endpos, cameras[cam]->targetOrigin, predictedNextTarget);
		VectorAdd (predictedNextTarget, tr.endpos, predictedNextTarget);
		tr = CM_BoxTrace (cl.refdef.vieworg, predictedNextTarget, mins, maxs, 0, MASK_SOLID);
		VectorSubtract (tr.endpos, predictedNextTarget, predictedOffset);
		VectorScale (predictedOffset, 0.63, predictedOffset);

		// Influence the targetOrigin based on our prediction
		VectorAdd (cameras[cam]->targetOrigin, predictedOffset, cameras[cam]->targetOrigin);*/
	}

	tr = CamEngine_Trace(cameras[cam]->origin, cameras[cam]->targetOrigin, cameras[cam]->radius, MASK_CAMERA);

	/*
	NeVo - now make sure the path from the camera origin to the
	camera targetOrigin is clear, and adjust the camera targetOrigin appropriately
	*/
	while (tr.fraction != 1.0)
	{
		VectorSubtract(cameras[cam]->targetOrigin, cameras[cam]->origin, midpoint);
		VectorScale(midpoint, 0.5, midpoint);
		VectorAdd(cameras[cam]->origin, midpoint, midpoint);
		tr = CamEngine_Trace(cameras[cam]->origin, midpoint, cameras[cam]->radius, MASK_CAMERA);
		if (!VectorCompare(tr.endpos, midpoint))
			VectorCopy(midpoint, cameras[cam]->targetOrigin);
	}
}

/*
==============
CamEngine_Damp

Damp camera motion
==============
*/
void CamEngine_Damp(camera_t cam)
{
	vec3_t displacement;
	vec3_t velocity;
	float displen, magnitude;

	// Return if damping is disabled
	if (!camdevice.GetOption(cam, CAMMODE_DAMPED))
	{
		VectorCopy(cameras[cam]->targetOrigin, cameras[cam]->origin);
		return;
	}
	else if (VectorCompare(cameras[cam]->targetOrigin, cameras[cam]->origin))
		return;		// Return if identical

	// Update Displacement
	VectorSubtract(cameras[cam]->origin, cameras[cam]->targetOrigin, displacement);
	displen = VectorLength(displacement);

	// Update Velocity
	VectorSubtract(cameras[cam]->oldTargetOrigin, cameras[cam]->targetOrigin, velocity);
	VectorScale(velocity, cls.frametime * VELOCITY_DAMP, velocity);

	// Update Magnitude
	magnitude = SPRING_CONST *
		(cameras[cam]->tether - displen) +
		DAMP_CONST *
		(DotProduct(displacement, velocity) /
			displen);

	// Update Camera
	VectorNormalize(displacement);
	VectorScale(displacement, magnitude * cls.frametime, displacement);
	VectorAdd(cameras[cam]->origin, displacement, cameras[cam]->origin);
}

void CamEngine_DirectView(camera_t cam)
{
	VectorCopy(cameras[cam]->targetOrigin, cameras[cam]->origin);

	vec3_t difference;
	VectorSubtract(cameras[cam]->lookAtTarget, cl.refdef.vieworg, difference);
	float dist = sqrtf(difference[0] * difference[0] + difference[1] * difference[1]);

	cl.refdef.viewangles[PITCH] = -glm_deg(atan2(difference[2], dist));
	//cl.refdef.viewangles[YAW] -= cl_thirdPersonAngle->value;
}

/*
===============
CamEngine_Think

Do some real work
===============
*/
void CamEngine_Think(camera_t cam)
{
	// Primary update
	CamEngine_UpdatePrimary(cam);

	// Cipher zoom and magnification
	CamEngine_Zoom(cam);

	// Update target position
	CamEngine_UpdateTarget(cam);

	// Update View
	if (cam == camdevice.GetCurrentCam())
	{
		// Dampen motion
		CamEngine_Damp(cam);

		// Adjust FOV
		CamEngine_FOV(cam);

		// Adjust camera position
		CamEngine_Clip(cam);

		// Align view
		CamEngine_DirectView(cam);

		// Calculate Alpha
		CamEngine_CalcAlpha(cam);

		// Kick the camera
		CamEngine_Kick(cam);

		VectorCopy(cameras[cam]->origin, cl.refdef.vieworg);
	}
}

/*
===============
CamEngine_Trace

Run a trace for collision detection
===============
*/
trace_t CamEngine_Trace(vec3_t start, vec3_t end, float size, int contentmask)
{
	// This stuff is for Box Traces
	vec3_t maxs, mins;

	VectorSet(maxs, size, size, size);
	VectorSet(mins, -size, -size, -size);

	return CM_BoxTrace(start, end, mins, maxs, 0, contentmask);
}

/*
=======================
CamEngine_UpdatePrimary

Update vieworg to first person position
Note: at one point, this used to be CL_CalcViewValues
=======================
*/
void CamEngine_UpdatePrimary(camera_t cam)
{
	int			i = 0;
	float		lerp = 0.0f, backlerp = 0.0f;
	centity_t* ent = NULL;
	frame_t* oldframe = NULL;
	player_state_t* ps = NULL, * ops = NULL;

	// find the previous frame to interpolate from
	ps = &cl.frame.playerstate;

	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or invalid
	ops = &oldframe->playerstate;

	// see if the player entity was teleported this frame
	if (Q_fabs(ops->pmove.origin[0] - ps->pmove.origin[0]) > TELEPORT_DIST
		|| Q_fabs(ops->pmove.origin[1] - ps->pmove.origin[1]) > TELEPORT_DIST
		|| Q_fabs(ops->pmove.origin[2] - ps->pmove.origin[2]) > TELEPORT_DIST)
		ops = ps;		// don't interpolate

	ent = &cl_entities[cl.playernum + 1];
	lerp = cl.lerpfrac;

	// calculate the origin - modified by Psychospaz for 3rd person camera
	if ((cl_predict->value) && !(cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{	// use predicted values
		unsigned	delta = 0;

		backlerp = 1.0 - lerp;
		for (i = 0; i < 3; i++)
		{
			cl.refdef.vieworg[i] = cl.predicted_origin[i] + ops->viewoffset[i]
				+ cl.lerpfrac * (ps->viewoffset[i] - ops->viewoffset[i])
				- backlerp * cl.prediction_error[i];

			if (cameras[cam]->type == CAMTYPE_THIRDPERSON)	// NeVo - fix annoying bug
			{
				//this smooths out platform riding
				cl.predicted_origin[i] -= backlerp * cl.prediction_error[i];
			}
		}

		// smooth out stair climbing
		delta = cls.realtime - cl.predicted_step_time;
		if (delta < 100)
		{
			cl.refdef.vieworg[2] -= cl.predicted_step * (100 - delta) * 0.01;

			if (cameras[cam]->type == CAMTYPE_THIRDPERSON)	// NeVo - fix annoying bug
			{
				cl.predicted_origin[2] -= cl.predicted_step * (100 - delta) * 0.01;
			}
		}
	}
	else
	{	// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.vieworg[i] = ops->pmove.origin[i] * 0.125 + ops->viewoffset[i]
			+ lerp * (ps->pmove.origin[i] * 0.125 + ps->viewoffset[i]
				- (ops->pmove.origin[i] * 0.125 + ops->viewoffset[i]));
	}

	if ((cameras[cam]->type == CAMTYPE_FIRSTPERSON) || (cameras[cam]->type == CAMTYPE_THIRDPERSON))
	{
		VectorCopy(cl.refdef.vieworg, cameras[cam]->origin);
		
		// Get our target viewpoint
		CamEngine_CalcLookat(cam, ps, ops);
	}

	// don't interpolate blend color
	for (i = 0; i < 4; i++)
		cl.refdef.blend[i] = ps->blend[i];

	// Return if we aren't active
	if (cam != camdevice.GetCurrentCam())
		return;

	// If the camera is a first person camera, draw the vweap
	if ((camtype_t)camdevice.GetOption(cam, CAM_TYPE) == CAMTYPE_FIRSTPERSON)
	{
		CL_AddViewWeapon(ps, ops);
	}
}

/*
======================
CamEngine_UpdateTarget

Update targetOrigin position
======================

*/

void CamEngine_UpdateTarget(camera_t cam)
{
	trace_t tr;
	float sphereRadius = 0.0f, angleRad = 0.0f, forwardScale = 0.0f, rightScale = 0.0f;
	
	if (cameras[cam]->type == CAMTYPE_FIRSTPERSON)
	{
		VectorCopy(cl.refdef.vieworg, cameras[cam]->targetOrigin);
		VectorCopy(cl.refdef.vieworg, cameras[cam]->oldTargetOrigin);
		return;
	}

	// Set old target origin
	VectorCopy(cameras[cam]->targetOrigin, cameras[cam]->oldTargetOrigin);

	// Calculate relative target position
	sphereRadius = cameras[cam]->currentdist;

	// Prevent the camera from being too far away
	if (fabs(sphereRadius) > cameras[cam]->maxdist)
	{
		sphereRadius = cameras[cam]->maxdist;
	}

	angleRad = DEG2RAD(cl_thirdPersonAngle->value);

	forwardScale = -sphereRadius * cosf(angleRad);
	rightScale = -sphereRadius * sinf(angleRad * 0.5f);

	cl.refdef.viewangles[PITCH] *= 0.5f;

	AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

	// Update targetOrigin using head origin
	VectorMA(cl.refdef.vieworg, forwardScale, cl.v_forward, cameras[cam]->targetOrigin);
	VectorMA(cameras[cam]->targetOrigin, rightScale, cl.v_right, cameras[cam]->targetOrigin);

	// Clip the targetOrigin
	tr = CamEngine_Trace(cameras[cam]->origin, cameras[cam]->targetOrigin, cameras[cam]->radius, MASK_CAMERA);

	if (tr.fraction != 1.0f)
	{
		VectorCopy(tr.endpos, cameras[cam]->targetOrigin);
	}

	// Smart collision avoidance
	CamEngine_SmartTarget(cam);
}

/*
==============
CamEngine_Zoom

Zoom camera in or out
==============
*/
void CamEngine_Zoom(camera_t cam)
{
	vec3_t	displacement;

	if (cameras[cam]->type == CAMTYPE_THIRDPERSON)
	{
		cameras[cam]->mindist = 3.0f;
		cameras[cam]->maxdist = cl_thirdPersonDist->value + cameras[cam]->mindist;
		cameras[cam]->normdist = (cl_thirdPersonDist->value * 0.5f) + cameras[cam]->mindist;
	}

	if (cameras[cam]->currentdist < 0.0f)
	{
		cameras[cam]->currentdist = 0.0f;
	}
	if (cameras[cam]->currentdist > cameras[cam]->maxdist)
	{
		cameras[cam]->currentdist = cameras[cam]->maxdist;
	}

	// Return if we aren't active
	if (cam != camdevice.GetCurrentCam())
		return;

	// Adjust position
	if (cameras[cam]->zoomable)
	{
		switch (camdevstate.zoommode)
		{
		case CAMZOOM_AUTO_IN:
			cameras[cam]->currentdist -= AUTOZOOM_UPF;
			if (cameras[cam]->currentdist < cameras[cam]->mindist)
			{
				camdevstate.zoommode = CAMZOOM_NONE;
				cameras[cam]->currentdist = cameras[cam]->mindist;

				camdevice.Activate(QCAMERA0);
			}
			break;
		case CAMZOOM_AUTO_OUT:
			cameras[cam]->currentdist += AUTOZOOM_UPF;
			if (cameras[cam]->currentdist > cameras[cam]->normdist)
			{
				camdevstate.zoommode = CAMZOOM_NONE;
				cameras[cam]->currentdist = cameras[cam]->normdist;
			}
			break;
		case CAMZOOM_MANUAL_IN:
			cameras[cam]->currentdist -= MANUALZOOM_UPF;
			camdevstate.zoommode = CAMZOOM_NONE;
			if ((cameras[cam]->currentdist - FLT_EPSILON) < (cameras[cam]->mindist + FLT_EPSILON))
			{
				cameras[cam]->currentdist = cameras[cam]->mindist;
				camdevice.Activate(QCAMERA0);
			}
			break;
		case CAMZOOM_MANUAL_OUT:
			cameras[cam]->currentdist += MANUALZOOM_UPF;
			camdevstate.zoommode = CAMZOOM_NONE;
			if (cameras[cam]->currentdist > cameras[cam]->maxdist)
				cameras[cam]->currentdist = cameras[cam]->maxdist;
			break;
		case CAMZOOM_NONE:
		default:
			camdevstate.zoommode = CAMZOOM_NONE;
			break;
		}
	}

	// Adjust magnification
	if (cameras[cam]->magnifiable)
	{
		switch (camdevstate.zoommode)
		{
		case CAMZOOM_AUTO_IN:
			cameras[cam]->magnification += AUTOZOOM_UPF;
			if (cameras[cam]->magnification >= cameras[cam]->normmagnification)
			{
				camdevstate.zoommode = CAMZOOM_NONE;
				cameras[cam]->magnification = cameras[cam]->normmagnification;
			}
			break;
		case CAMZOOM_AUTO_OUT:
			cameras[cam]->magnification -= AUTOZOOM_UPF;
			if (cameras[cam]->magnification <= cameras[cam]->minmagnification)
			{
				camdevstate.zoommode = CAMZOOM_NONE;
				cameras[cam]->magnification = cameras[cam]->minmagnification;
				camdevice.ActivateLast();
			}
			break;
		case CAMZOOM_MANUAL_IN:
			cameras[cam]->magnification += MANUALZOOM_UPF;
			camdevstate.zoommode = CAMZOOM_NONE;
			if (cameras[cam]->magnification >= cameras[cam]->maxmagnification)
				cameras[cam]->magnification = cameras[cam]->maxmagnification;
			break;
		case CAMZOOM_MANUAL_OUT:
			cameras[cam]->magnification -= MANUALZOOM_UPF;
			camdevstate.zoommode = CAMZOOM_NONE;
			if (cameras[cam]->magnification <= cameras[cam]->minmagnification)
				cameras[cam]->magnification = cameras[cam]->minmagnification;
			break;
		case CAMZOOM_NONE:
		default:
			camdevstate.zoommode = CAMZOOM_NONE;
			break;
		}
	}
}

/*
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
					Camera Export Functions
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
This section contains the camera export functions.
*/

/*
==============
Cam_InitSystem

Initialize the camera system
==============
*/
void Cam_InitSystem(void)
{
	int i;

	/*		Initialize camera device to system		*/
	camdevice.Activate = CamSystem_Activate;
	camdevice.ActivateLast = CamSystem_ActivateLast;
	camdevice.CreateCam = CamSystem_CreateCam;
	camdevice.GetCurrentCam = CamSystem_ReturnCam;
	camdevice.SetOption = CamSystem_SetOption;
	camdevice.GetOption = CamSystem_GetOption;
	camdevice.ShutdownCamera = CamSystem_ShutdownCam;
	camdevice.ShutdownCameras = CamSystem_ShutdownCams;
	camdevice.Think = CamSystem_Think;
	camdevice.ToChaseCam = CamSystem_ToChase;
	camdevice.ToEyeCam = CamSystem_ToEyes;
	camdevice.ZoomIn = CamSystem_ZoomIn;
	camdevice.ZoomOut = CamSystem_ZoomOut;

	// Register Cvars
	cl_thirdPerson = Cvar_Get("cl_thirdPerson", "1", CVAR_ARCHIVE);
	cl_thirdPersonDist = Cvar_Get("cl_thirdPersonDist", "85", CVAR_ARCHIVE);
	cl_thirdPersonAngle = Cvar_Get("cl_thirdPersonAngle", "30", CVAR_ARCHIVE);
	cl_thirdPersonDamp = Cvar_Get("cl_thirdPersonDamp", "1", CVAR_ARCHIVE);

	// Initialize Camera Device State
	camdevstate.ActiveCamera = QCAMERA0;
	for (i = 0; i < MAX_CAMERAS; i++)
		camdevstate.DefinedCameras[i] = false;
	camdevstate.LastCamera = QCAMERA0;
	camdevstate.locked = false;
	camdevstate.maxvertangle = 90;
	camdevstate.viewermodelalpha = 1.0;
	camdevstate.zoommode = CAMZOOM_NONE;

	// Create default cameras, set current camera to first person
	camdevice.CreateCam(QCAMERA0, CAMTYPE_FIRSTPERSON);
	camdevice.CreateCam(QCAMERA1, CAMTYPE_THIRDPERSON);
	camdevice.Activate(QCAMERA0);

	// Add commands
	Cmd_AddCommand("camtoggle", Cam_Toggle_f);
	Cmd_AddCommand("camzoomin", Cam_ZoomIn_f);
	Cmd_AddCommand("camzoomout", Cam_ZoomOut_f);
	Cmd_AddCommand("camreset", Cam_Reset_f);

	camdevstate.Initialized = true;
}

/*
============
Cam_Shutdown

Shutdown camera system
============
*/
void Cam_Shutdown(void) {
	Cmd_RemoveCommand("camtoggle");
	Cmd_RemoveCommand("camzoomin");
	Cmd_RemoveCommand("camzoomout");
	Cmd_RemoveCommand("camreset");

	// Shutdown cameras
	if (camdevstate.Initialized)
	{
		camdevice.ShutdownCameras();
	}

	camdevstate.Initialized = false;
}

/*
================
Cam_UpdateEntity

Update the player entity
================
*/
void Cam_UpdateEntity(entity_t* ent)
{
	int i;

	// If this entity is the player
	if (ent->flags & RF_VIEWERMODEL)
	{
		// Update the player's origin 
		for (i = 0; i < 3; i++)
			ent->oldorigin[i] = ent->origin[i] = cl.predicted_origin[i];

		// If not a first person camera
		if (!(cameras[camdevice.GetCurrentCam()]->type == CAMTYPE_FIRSTPERSON))
		{
			// Update player model alpha
			CamEngine_AddEntAlpha(ent);
			ent->flags &= ~RF_VIEWERMODEL;
		}
	}
}

/*
||||||||||||||||||||||||||||||||||||||||||||||
				Console Commands
||||||||||||||||||||||||||||||||||||||||||||||
This section contains the console commands.
These only affect the active camera.
*/


/*
===========
Cam_Reset_f

Reset camera
===========
*/
void Cam_Reset_f(void)
{
	int cam;

	if (!camdevstate.Initialized)
		return;

	cam = camdevice.GetCurrentCam();

	// Return if locked
	if (camdevstate.locked)
		return;

	camdevstate.DefinedCameras[cam] = false;
	camdevice.CreateCam(cam, (camtype_t)camdevice.GetOption(cam, CAM_TYPE));
}

/*
=================
Cam_Toggle_f

Toggle Third Person Camera
=================
*/
void Cam_Toggle_f(void)
{
	int cam = camdevice.GetCurrentCam();

	// Return if locked
	if (camdevstate.locked)
		return;

	// NeVo - finally fixed this dumb ass bug
	if (cameras[cam]->type == CAMTYPE_FIRSTPERSON)
	{
		// We are in first person, so switch to chase camera
		camdevice.ToChaseCam();
		return;
	}
	else if (cameras[cam]->type == CAMTYPE_THIRDPERSON)
	{
		// We are a chase camera, so switch to first person
		camdevice.ToEyeCam();
		return;
	}
	else
	{
		// Otherwise just switch to first person
		camdevice.Activate(QCAMERA0);
		return;
	}
}

/*
=================
Cam_ZoomIn_f

Zoom camera in
=================
*/
void Cam_ZoomIn_f(void) {
	// Return if locked
	if (camdevstate.locked)
		return;
	camdevice.ZoomIn();
}

/*
==================
Cam_ZoomOut_f

Zoom camera out
==================
*/
void Cam_ZoomOut_f(void) {
	// Return if locked
	if (camdevstate.locked)
		return;
	camdevice.ZoomOut();
}