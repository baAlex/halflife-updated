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


static constexpr int WHITE[3] = {255, 255, 255};
static constexpr int GREY[3] = {92, 92, 92};
static constexpr int RED[3] = {200, 0, 0};

static SCREENINFO_s s_screen;
static int s_margin = 15;


class Health
{
	static constexpr int HEALTH_PER_BLOCK = 5;
	static constexpr int HEALTH_PER_SEPARATOR = 25;
	static constexpr int CRITICAL_THRESHOLD = 25;
	static constexpr int HIGHLIGHT_DURATION = 3; // In seconds, too long and it becomes misleading/non-legible

	HSPRITE m_block;
	HSPRITE m_separator;

	int m_block_w;
	int m_separator_w;
	int m_y;

	int m_highlight_time;
	int m_previous_health;

  public:
	void Initialize()
	{
		m_block = gEngfuncs.pfnSPR_Load("sprites/480-health-block.spr");
		m_separator = gEngfuncs.pfnSPR_Load("sprites/480-health-separator.spr");

		m_block_w = gEngfuncs.pfnSPR_Width(m_block, 0);
		m_separator_w = gEngfuncs.pfnSPR_Width(m_separator, 0);

		m_y = s_screen.iHeight - s_margin -
		      Ic::Max(gEngfuncs.pfnSPR_Height(m_block, 0), gEngfuncs.pfnSPR_Height(m_separator, 0));

		SoftInitialize();
	}

	void SoftInitialize()
	{
		// We don't want these to be carried between matches, saved games or demos
		m_highlight_time = 0;
		m_previous_health = 0;
	}

	void Draw(float time)
	{
		(void)time;
		struct Rect rect; // pfnSPR_Draw() seems intended to return something... but is all zeros

		const int health = (Ic::PlayerHealth() > CRITICAL_THRESHOLD)
		                       ? (Ic::PlayerHealth() - HEALTH_PER_BLOCK + 1) // Rounding, otherwise it feels like
		                       : Ic::PlayerHealth();                         // the Hud isn't registering damage
		const int max_health = Ic::PlayerMaxHealth();

		// Draw blocks
		if (1)
		{
			int x = s_margin;
			int i = 0;

			// Full health
			if (Ic::PlayerHealth() >= CRITICAL_THRESHOLD)
				gEngfuncs.pfnSPR_Set(m_block, WHITE[0], WHITE[1], WHITE[2]);
			else
				gEngfuncs.pfnSPR_Set(m_block, RED[0], RED[1], RED[2]);

			for (; i < health; i += HEALTH_PER_BLOCK)
			{
				if (i > 0 && (i % HEALTH_PER_SEPARATOR) == 0)
					x += m_separator_w;

				gEngfuncs.pfnSPR_DrawHoles(0, x + m_block_w * (i / HEALTH_PER_BLOCK), m_y, &rect);
			}

			// Highlighted health
			if (m_previous_health > health && Ic::PlayerHealth() >= CRITICAL_THRESHOLD)
			{
				gEngfuncs.pfnSPR_Set(m_block, RED[0], RED[1], RED[2]);

				for (; i < m_previous_health; i += HEALTH_PER_BLOCK)
				{
					if (i > 0 && (i % HEALTH_PER_SEPARATOR) == 0)
						x += m_separator_w;

					gEngfuncs.pfnSPR_DrawHoles(0, x + m_block_w * (i / HEALTH_PER_BLOCK), m_y, &rect);
				}
			}

			// Empty health
			gEngfuncs.pfnSPR_Set(m_block, GREY[0], GREY[1], GREY[2]);

			for (; i < max_health; i += HEALTH_PER_BLOCK)
			{
				if (i > 0 && (i % HEALTH_PER_SEPARATOR) == 0)
					x += m_separator_w;

				gEngfuncs.pfnSPR_DrawHoles(1, x + m_block_w * (i / HEALTH_PER_BLOCK), m_y,
				                           &rect); // Notice that's frame 1
			}
		}

		// Draw separators
		if (1)
		{
			const int space = (m_block_w * (HEALTH_PER_SEPARATOR / HEALTH_PER_BLOCK)) + m_separator_w;

			int x = s_margin - m_separator_w;
			int i = HEALTH_PER_SEPARATOR; // Skip first one

			// Full health
			if (Ic::PlayerHealth() >= CRITICAL_THRESHOLD)
				gEngfuncs.pfnSPR_Set(m_separator, WHITE[0], WHITE[1], WHITE[2]);
			else
				gEngfuncs.pfnSPR_Set(m_separator, RED[0], RED[1], RED[2]);

			for (; i < health; i += HEALTH_PER_SEPARATOR)
				gEngfuncs.pfnSPR_DrawHoles(0, x + space * (i / HEALTH_PER_SEPARATOR), m_y, &rect);

			// Highlighted health
			if (m_previous_health > health && Ic::PlayerHealth() >= CRITICAL_THRESHOLD)
			{
				// Critical health and highlight, both being red, is confusing. So it's one
				// or the other (that's why the condition)
				gEngfuncs.pfnSPR_Set(m_separator, RED[0], RED[1], RED[2]);

				for (; i < m_previous_health; i += HEALTH_PER_BLOCK)
					gEngfuncs.pfnSPR_DrawHoles(0, x + space * (i / HEALTH_PER_SEPARATOR), m_y, &rect);
			}

			// Empty health
			gEngfuncs.pfnSPR_Set(m_separator, GREY[0], GREY[1], GREY[2]);

			for (; i < max_health; i += HEALTH_PER_SEPARATOR)
				gEngfuncs.pfnSPR_DrawHoles(0, x + space * (i / HEALTH_PER_SEPARATOR), m_y, &rect);
		}

		// Update this bad boy
		if (m_previous_health != health)
		{
			if (static_cast<int>(time) > m_highlight_time)
				m_previous_health = health;
		}
		else
			m_highlight_time = static_cast<int>(time) + HIGHLIGHT_DURATION;
	}
};


class Crosshair
{
	// Seems like I'm in a good direction, crosshair here looks identical to the
	// on of Day of Defeat (including bleeding, blur). The one in Counter Strike tho,
	// seems to be using the TriangleApi, as is quite sharp and thin.

	static constexpr int PAD = 2; // Is baked on the sprite, intended to avoid OpenGl bleeding
	static constexpr int GAP = 9;

	HSPRITE m_horizontal;
	HSPRITE m_vertical;

	int h_w;
	int v_h;

  public:
	void Initialize()
	{
		m_horizontal = gEngfuncs.pfnSPR_Load("sprites/480-crosshair-h.spr");
		m_vertical = gEngfuncs.pfnSPR_Load("sprites/480-crosshair-v.spr");

		h_w = gEngfuncs.pfnSPR_Width(m_horizontal, 0);
		v_h = gEngfuncs.pfnSPR_Height(m_vertical, 0);
	}

	void SoftInitialize() {}

	void Draw(float time)
	{
		(void)time;
		struct Rect rect;

		gEngfuncs.pfnSPR_Set(m_horizontal, WHITE[0], WHITE[1], WHITE[2]);
		gEngfuncs.pfnSPR_DrawHoles(0, s_screen.iWidth / 2 + GAP, s_screen.iHeight / 2 - PAD, &rect);
		gEngfuncs.pfnSPR_DrawHoles(1, s_screen.iWidth / 2 - GAP - h_w, s_screen.iHeight / 2 - PAD, &rect);

		gEngfuncs.pfnSPR_Set(m_vertical, WHITE[0], WHITE[1], WHITE[2]);
		gEngfuncs.pfnSPR_DrawHoles(0, s_screen.iWidth / 2 - PAD, s_screen.iHeight / 2 + GAP, &rect);
		gEngfuncs.pfnSPR_DrawHoles(1, s_screen.iWidth / 2 - PAD, s_screen.iHeight / 2 - GAP - v_h, &rect);
	}
};


static Health s_health;
static Crosshair s_crosshair;


void Ic::HudVideoInitialise()
{
	// «Called when the game initializes and whenever the vid_mode is changed»
	// «Called at start and end of demos to restore to "non" HUD state»
	// Valve (cl_dll/cdll_int.cpp)

	// We are called directly by the engine:
	//    Ic::HudVideoInitialise() <- HUD_VidInit()
	//    Ic::HudVideoInitialise() <- HUD_Reset()

	s_screen.iSize = sizeof(SCREENINFO_s); // Silly versioning thing
	gEngfuncs.pfnGetScreenInfo(&s_screen);

	s_health.Initialize();
	s_crosshair.Initialize();
}


void Ic::HudInitialise()
{
	// «Called whenever the client connects to a server»
	// Valve (cl_dll/cdll_int.cpp)

	// As above:
	//    Ic::HudInitialise() <- HUD_Init()

	s_health.SoftInitialize();
	s_crosshair.SoftInitialize();
}


void Ic::HudDraw(float time)
{
	// «called every screen frame to redraw the HUD.»
	// Valve (cl_dll/cdll_int.cpp)

	// As above:
	//    Ic::HudDraw() <- HUD_Redraw()

	if (Ic::IsPlayerDead() == true)
		return;

	s_health.Draw(time);
	s_crosshair.Draw(time);
}
