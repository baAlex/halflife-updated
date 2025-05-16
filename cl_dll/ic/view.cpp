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
#include "ic/base.hpp"


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
// Is not my idea to re implement everything, what is here is the bare minimum
// to make the engine work. Notable absences are: spectator functions (too much trouble
// set a server to test them), interpolate towards an ideal pitch mandated by the server
// (V_DriftPitch), and view-weapons snapping when auto-aiming.

// On the other hand all aesthetic choices are different. For example I don't think that
// keep player head above water waves is needed, nor all the cameras tilts.


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


#define SMOOTH_Z
#define SWAY
#define LEAN
#define BOB
#define BREATH

#ifdef SMOOTH_Z
static float s_smooth_z;
static constexpr float SMOOTH_Z_AMOUNT = 20.0f; // Tested against stairs in crossfire
static constexpr float SMOOTH_Z_AMOUNT_AIR = 40.0f;
#endif

#ifdef SWAY
static float s_sway[2];
static constexpr float SWAY_AMOUNT[2] = {1.75f * 7.0f, 1.75f * 5.0f}; // Slow to make it obvious that is on purpose
static constexpr float SWAY_OUTSIDE_RANGE[2] = {30.0f, 40.0f};
static constexpr float SWAY_INSIDE_RANGE[2] = {30.0f / 2.0f, 40.0f / 2.0f};
#endif

#ifdef LEAN
static Vector s_lean;
static constexpr Vector LEAN_AMOUNT = {8.0f, 6.0f, 6.0f};
static constexpr float LEAN_SMOOTH = 10.0f;
#endif

#ifdef BOB
static float s_walk_speed;
static constexpr float DUCK_MULTIPLY_SPEED = 0.5f;
static constexpr float SPEED_THRESHOLD = 10.0f;
static constexpr float MAXIMUM_WALK_SPEED = 400.0f;
static constexpr float WALK_SMOOTH = 5.0f;

static float s_bob[2];
static constexpr float BOB_AMOUNT[2] = {1.0f * 0.3f, 1.0f * 0.2f};
static constexpr float BOB_SPEED[2] = {1.0f * 12.0f, 1.0f * 6.0f};
#endif

#ifdef BREATH
static Vector s_breath = {0.589, 1.123, 2.333f}; // Some random phases
static constexpr Vector BREATH_AMOUNT = {0.33f * 1.0f, 0.33f * 1.0f, 0.33f * 1.0f};
static constexpr Vector BREATH_SPEED = {1.0f / 4.4f, 1.0f / 3.5f, 1.0f / 2.6f};
#endif


#ifdef BOB
static float sWalkSpeed(const struct ref_params_s* in)
{
	// Speed that player inputs in (keyboard/controller)
	float speed = sqrtf((in->cmd->forwardmove * in->cmd->forwardmove) + (in->cmd->sidemove * in->cmd->sidemove));

	// Gordon Freeman actual physical speed, it may be because player
	// input, because an explosion, a slope, friction, inertia, who knows
	const float actual_vel = sqrtf(in->simvel[0] * in->simvel[0] + in->simvel[1] * in->simvel[1]);

	// Do no count if in air, neither if Freeman speed is minimal,
	// (player input is blind to Freeman being stuck)
	if (in->onground == 0 || actual_vel < SPEED_THRESHOLD)
		speed = 0.0f;

	// Crouching speed isn't accounted by engine (even if walk/sprint are)
	if (in->cmd->buttons & IN_DUCK)
		speed *= DUCK_MULTIPLY_SPEED; // We do it ourselves

	// Normalize (0, 1), not perfect but will yield something close to 1
	speed /= MAXIMUM_WALK_SPEED;
	speed = Ic::Clamp(speed, 0.0f, 1.5f); // In case something went really wrong

	// Smooth, all conditional above are hard ones
	s_walk_speed = Ic::HolmerMix(speed, s_walk_speed, WALK_SMOOTH, in->frametime);
	// gEngfuncs.Con_Printf("%f, %f\n", speed, s_walk_speed);

	return s_walk_speed;
}
#endif


static void sNormalView(struct ref_params_s* in_out)
{
	const float dt = in_out->frametime; // Misleading name

	// View origin and angles, copy from predicted values
	VectorCopy(in_out->simorg, in_out->vieworg);
	VectorAdd(in_out->viewheight, in_out->vieworg, in_out->vieworg);

	if (in_out->health > 0) // Not angles if we are dead
	{
		VectorCopy(in_out->cl_viewangles, in_out->viewangles);
	}

#ifdef SMOOTH_Z
	{
		const float rough_z = in_out->vieworg[2];

		if (in_out->onground != 0)
			s_smooth_z = Ic::HolmerMix(rough_z, s_smooth_z, SMOOTH_Z_AMOUNT, dt);
		else
			s_smooth_z = Ic::HolmerMix(rough_z, s_smooth_z, SMOOTH_Z_AMOUNT_AIR, dt);

		// 30 is enough to jump and crouch without trigger the clamp
		if (0 && (s_smooth_z < rough_z - 30.0f || s_smooth_z > rough_z + 30.0f))
			gEngfuncs.Con_Printf("Smooth is lagging behind! (%f)\n", in_out->time);

		s_smooth_z = Ic::Clamp(s_smooth_z, rough_z - 30.0f, rough_z + 30.0f);
		in_out->vieworg[2] = s_smooth_z;
	}
#endif

	// No idea what this does aside from an offset thingie banned
	// in multiplayer. Is marked as output in 'ref_params_s' tho,
	// so maybe the engine is using it for render purposes
	AngleVectors(in_out->cl_viewangles, in_out->forward, in_out->right, in_out->up);

	// Weapon model
	cl_entity_t* view_model = gEngfuncs.GetViewModel();
	if (view_model != nullptr)
	{
		// Copy origin and angles, same as view
		VectorCopy(in_out->vieworg, view_model->origin);

		VectorCopy(in_out->cl_viewangles, view_model->angles);
		view_model->angles[PITCH] = -view_model->angles[PITCH]; // Pitch is inverted [a]

		// From here a lot of modulations follow

#ifdef SWAY
		{
			// Smooth
			s_sway[0] = Ic::AnglesHolmerMix(-in_out->cl_viewangles[0], s_sway[0], SWAY_AMOUNT[0], dt);
			s_sway[1] = Ic::AnglesHolmerMix(in_out->cl_viewangles[1], s_sway[1], SWAY_AMOUNT[1], dt);

			// Clamp, don't let the sway lag too much
			s_sway[0] = Ic::ClampAroundCentre(s_sway[0], -in_out->cl_viewangles[0], SWAY_OUTSIDE_RANGE[0]);
			s_sway[1] = Ic::ClampAroundCentre(s_sway[1], in_out->cl_viewangles[1], SWAY_OUTSIDE_RANGE[1]);

			// Smooth again, 1 pole filter this time
			view_model->angles[0] = Ic::AnglesMix(-in_out->cl_viewangles[0], s_sway[0], 0.5f);
			view_model->angles[1] = Ic::AnglesMix(in_out->cl_viewangles[1], s_sway[1], 0.5f);
			view_model->angles[2] = 0.0f;
		}
#endif

#ifdef LEAN
		{
			Vector temp;
			VectorScale(in_out->simvel, 1.0f / MAXIMUM_WALK_SPEED, temp);

			s_lean[0] = Ic::HolmerMix(LEAN_AMOUNT[0] * DotProduct(in_out->forward, temp), s_lean[0], LEAN_SMOOTH, dt);
			s_lean[1] = Ic::HolmerMix(LEAN_AMOUNT[1] * DotProduct(in_out->right, temp), s_lean[1], LEAN_SMOOTH, dt);
			s_lean[2] = Ic::HolmerMix(LEAN_AMOUNT[2] * DotProduct(in_out->up, temp), s_lean[2], LEAN_SMOOTH, dt);

			view_model->angles[0] -= (s_lean[0] > 0.0f) ? s_lean[0] : s_lean[0] / 2.0f; // Less when backwards
			view_model->angles[2] += s_lean[1];
			view_model->origin[2] -= Ic::Clamp(s_lean[2], -4.0f, 4.0f); // Please, don't go outside screen
		}
#endif

#ifdef BOB
		{
			float walk_speed = sWalkSpeed(in_out);

			VectorMA(view_model->origin, sinf(s_bob[0]) * walk_speed * BOB_AMOUNT[0], in_out->up, view_model->origin);
			VectorMA(view_model->origin, cosf(s_bob[1]) * walk_speed * BOB_AMOUNT[1], in_out->right,
			         view_model->origin);

			s_bob[0] = fmodf(s_bob[0] + dt * BOB_SPEED[0], M_PI * 2.0f);
			s_bob[1] = fmodf(s_bob[1] + dt * BOB_SPEED[1], M_PI * 2.0f);
		}
#endif

#ifdef BREATH
		{
			VectorMA(view_model->origin, sinf(s_breath[0]) * BREATH_AMOUNT[0], in_out->forward, view_model->origin);
			VectorMA(view_model->origin, sinf(s_breath[1]) * BREATH_AMOUNT[1], in_out->right, view_model->origin);
			VectorMA(view_model->origin, sinf(s_breath[2]) * BREATH_AMOUNT[2], in_out->up, view_model->origin);

			s_breath[0] = fmodf(s_breath[0] + dt * BREATH_SPEED[0], M_PI * 2.0f);
			s_breath[1] = fmodf(s_breath[1] + dt * BREATH_SPEED[1], M_PI * 2.0f);
			s_breath[2] = fmodf(s_breath[2] + dt * BREATH_SPEED[2], M_PI * 2.0f);
		}
#endif

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
