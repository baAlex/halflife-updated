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
#include "messages.hpp"
#include "ic/base.hpp"

#include "cl_dll.h"
#include "parsemsg.h"


static HSPRITE s_test_sprite;
static SCREENINFO_s s_screen;


void Ic::HudVideoInitialise()
{
	// «Called when the game initializes and whenever the vid_mode is changed»
	// «Called at start and end of demos to restore to "non" HUD state»
	// Valve (cl_dll/cdll_int.cpp)

	// We are called directly by the engine:
	//    Ic::HudVideoInitialise() <- HUD_VidInit()
	//    Ic::HudVideoInitialise() <- HUD_Reset()

	s_test_sprite = gEngfuncs.pfnSPR_Load("sprites/dot.spr");

	s_screen.iSize = sizeof(SCREENINFO_s); // Silly versioning thing
	gEngfuncs.pfnGetScreenInfo(&s_screen);
}


void Ic::HudInitialise()
{
	// «Called whenever the client connects to a server»
	// Valve (cl_dll/cdll_int.cpp)

	// As above:
	//    Ic::HudInitialise() <- HUD_Init()
}


void Ic::HudDraw(float time)
{
	struct Rect rect; // pfnSPR_Draw() seems intended to return something... but is all zeros
	(void)time;

	{
		const int w = gEngfuncs.pfnSPR_Width(s_test_sprite, 0);
		const int h = gEngfuncs.pfnSPR_Height(s_test_sprite, 0);

		const int health = Ic::PlayerHealth();
		const int y = s_screen.iHeight - h - 20;
		int x = 20;
		int i = 0;

		if (Ic::PlayerHealth() > 40)
			gEngfuncs.pfnSPR_Set(s_test_sprite, 31, 240, 150);
		else
			gEngfuncs.pfnSPR_Set(s_test_sprite, 255, 0, 0);

		for (; i < health; i += 10)
		{
			if (i > 0 && (i % 20) == 0)
				x += 4;
			gEngfuncs.pfnSPR_DrawAdditive(0, x + w * (i / 10), y, &rect);
		}

		gEngfuncs.pfnSPR_Set(s_test_sprite, 92, 92, 92);

		for (; i < 100; i += 10)
		{
			if (i > 0 && (i % 20) == 0)
				x += 4;
			gEngfuncs.pfnSPR_DrawAdditive(0, x + w * (i / 10), y, &rect);
		}
	}

	// gEngfuncs.pfnSPR_DrawAdditive(0, 512, 512, &rect);
}
