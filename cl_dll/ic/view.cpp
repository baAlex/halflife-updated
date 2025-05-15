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


extern int g_iUser1; // Defined in "cl_dll/vgui_TeamFortressViewport.cpp"


void Ic::ViewInitialize() {}

void Ic::ViewUpdate(struct ref_params_s* in_out)
{
	if (in_out->intermission != 0)
	{
		// V_CalcIntermissionRefdef(in_out);
	}
	else if (in_out->spectator != 0 || g_iUser1 != 0) // g_iUser true if in spectator mode
	{
		// V_CalcSpectatorRefdef(in_out);
	}
	else if (in_out->paused != 0)
	{
		// V_CalcNormalRefdef(in_out);
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
