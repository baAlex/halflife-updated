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

#include "hud.hpp"
#include "ic/base.hpp"

#include "cl_dll.h" // gEngfuncs


HSPRITE test;

void Ic::HudVideoInitialise()
{
	// «Called when the game initializes and whenever the vid_mode is changed»
	// «Called at start and end of demos to restore to "non" HUD state»
	// Valve (cl_dll/cdll_int.cpp)

	test = gEngfuncs.pfnSPR_Load("sprites/640hud7.spr");
}


void Ic::HudInitialise()
{
	// «Called whenever the client connects to a server»
	// Valve (cl_dll/cdll_int.cpp)
}


void Ic::HudDraw(float time)
{
	struct Rect rect; // pfnSPR_Draw() seems intended to return something... but is all zeros
	(void)time;

	gEngfuncs.pfnSPR_Set(test, 255, 0, 0);
	// gEngfuncs.pfnSPR_Draw(0, 512, 512, &rect);
	gEngfuncs.pfnSPR_DrawAdditive(0, 512, 512, &rect);
}
