/***
 *
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "view.hpp"


extern int g_iUser1;                     // Defined in "cl_dll/vgui_TeamFortressViewport.cpp"
extern Vector v_client_aimangles;        // Defined in "cl_dll/view.cpp"
extern Vector v_origin;                  // Ditto
extern Vector v_angles;                  // Ditto
extern Vector v_cl_angles;               // Ditto
extern Vector v_sim_org;                 // Ditto
extern int CL_IsThirdPerson();           // Defined in "cl_dll/in_camera.cpp"
extern void CL_CameraOffset(float* ofs); // Ditto


// Minimal view functions
// ======================
// Is no my idea to re implement everything, what is here is the bare minimum
// to make the engine work. Notable absences are, spectator functions (too much trouble
// set a server to test them), interpolate towards an ideal pitch mandated by the server
// (V_DriftPitch), smooth view in multiplayer, and view-weapons snapping when auto-aiming.

// On the other hand all aesthetic choices are different. For example I don't think that
// keep player head above water waves is truly needed, nor all the cameras tilts.


void Ic::ViewInitialize() {}


static void sIntermissionView(struct ref_params_s* in_out)
{
	// Disable weapon model
	cl_entity_t* view_model = gEngfuncs.GetViewModel();
	if (view_model != nullptr)
	{
		view_model->model = nullptr; // It seems that the engine does it by default
	}

	// View origin and angles, copy from predicted values
	if (gEngfuncs.IsSpectateOnly() == 0)
	{
		VectorCopy(in_out->simorg, in_out->vieworg);
		VectorAdd(in_out->viewheight, in_out->vieworg, in_out->vieworg);
		VectorCopy(in_out->cl_viewangles, in_out->viewangles);
	}
	else // Spectators use others values
	{
		VectorCopy(gHUD.m_Spectator.m_cameraOrigin, in_out->vieworg);
		VectorCopy(gHUD.m_Spectator.m_cameraAngles, in_out->viewangles);
	}

	// Used outside this file
	VectorCopy(in_out->viewangles, v_cl_angles);
	VectorCopy(in_out->vieworg, v_origin);
	VectorCopy(in_out->viewangles, v_angles);
}


static void sNormalView(struct ref_params_s* in_out)
{
	// View origin and angles, copy from predicted values
	VectorCopy(in_out->simorg, in_out->vieworg);
	VectorAdd(in_out->viewheight, in_out->vieworg, in_out->vieworg);

	VectorCopy(in_out->cl_viewangles, in_out->viewangles);

	// No idea what this does aside from an offset thingie banned
	// in multiplayer. Is marked as output in 'ref_params_s' tho,
	// so maybe the engine is using it for render purposes
	AngleVectors(in_out->cl_viewangles, in_out->forward, in_out->right, in_out->up);

	// Weapon model
	cl_entity_t* view_model = gEngfuncs.GetViewModel();
	if (view_model != nullptr)
	{
		// Copy origin and angles, same as view
		VectorCopy(in_out->simorg, view_model->origin);
		VectorAdd(in_out->viewheight, view_model->origin, view_model->origin);

		VectorCopy(in_out->cl_viewangles, view_model->angles);
		view_model->angles[PITCH] = -view_model->angles[PITCH]; // Pitch is inverted [a]

		// No idea, but is used outside this file,
		// should be done after all modulations
		VectorCopy(view_model->origin, view_model->curstate.origin);
		VectorCopy(view_model->origin, view_model->latched.prevorigin);
		VectorCopy(view_model->angles, view_model->curstate.angles);
		VectorCopy(view_model->angles, view_model->latched.prevangles);
	}

	// Third person model
	{
		cl_entity_t* ent;

		if (gEngfuncs.IsSpectateOnly() == 0)
			ent = gEngfuncs.GetLocalPlayer();
		else
			ent = gEngfuncs.GetEntityByIndex(g_iUser2);

		// Map view wider pitch to a shorter one for the model
		float pitch = in_out->viewangles[0];

		if (pitch > 180.0f)
			pitch -= 360.0f;
		else if (pitch < -180.f)
			pitch += 360;

		pitch /= -3.0; // Des-invert [a]

		ent->angles[0] = pitch;
		ent->curstate.angles[0] = pitch;
		ent->prevstate.angles[0] = pitch;
		ent->latched.prevangles[0] = pitch;
	}

	// Used outside this file,
	// -before third person camera changes-
	VectorCopy(in_out->viewangles, v_angles);
	VectorCopy(in_out->viewangles, v_client_aimangles);

	// Third person camera
	if (CL_IsThirdPerson() != 0)
	{
		Vector forward, right, up;
		Vector temp;
		float dist;

		CL_CameraOffset(temp); // Returns: pitch, yaw, dist
		dist = temp[2];        // Keep this
		temp[2] = 0.0f;        // Without 'dist' now is a proper vector

		AngleVectors(temp, forward, right, up);

		VectorMA(in_out->vieworg, -dist, forward, in_out->vieworg);
		VectorMA(in_out->vieworg, dist / 6.0f, right, in_out->vieworg);
		VectorCopy(temp, in_out->viewangles); // Overwrites previous value
	}

	// Used outside this file,
	// -after third person camera changes-
	VectorCopy(in_out->vieworg, v_origin);
}


void Ic::ViewUpdate(struct ref_params_s* in_out)
{
	if (in_out->intermission != 0)
	{
		sIntermissionView(in_out);
	}
	else if (in_out->spectator != 0 || g_iUser1 != 0) // g_iUser true if in spectator mode
	{
		// sSpectatorView(in_out);
	}
	else if (in_out->paused == 0)
	{
		sNormalView(in_out);
	}
}


void Ic::ViewApplyPunch(int axis, float punch)
{
	(void)axis;
	(void)punch;
	// Is up to us to save inputs and do something with them
}

void Ic::ViewGetChasePosition(int target, const float* cl_angles, float* out_origin, float* out_angles)
{
	(void)target;
	(void)cl_angles;
	(void)out_origin;
	(void)out_angles;
	// TODO, investigate further
}
