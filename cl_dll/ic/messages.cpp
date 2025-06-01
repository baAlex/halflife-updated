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

#include "messages.hpp"
#include "ic/base.hpp"

#include "cl_dll.h"
#include "parsemsg.h"


static int s_health;
static float s_accuracy[2];


static int sHealthChanges(const char* name, int size, void* pbuf)
{
	BEGIN_READ(pbuf, size);
	s_health = READ_SHORT();

	// gEngfuncs.Con_Printf("Health changes: %i\n", s_health);
	return 1;
}

static int sDamageReceive(const char* name, int size, void* pbuf)
{
	BEGIN_READ(pbuf, size);

	int armor = READ_BYTE();
	int damage = READ_BYTE();
	long damage_bits = READ_LONG();

	Vector vecFrom;
	for (int i = 0; i < 3; i++)
		vecFrom[i] = READ_COORD();

	gEngfuncs.Con_Printf("Damage receive, armor: %i, damage: %i\n", armor, damage);
	return 1;
}


void Ic::MessagesInitialise()
{
	// «Called whenever the client connects to a server»
	// Valve (cl_dll/cdll_int.cpp)

	// We are called directly by the engine:
	//    Ic::MessagesInitialise() <- HUD_Init()

	gEngfuncs.pfnHookUserMsg("Health", sHealthChanges);
	// gEngfuncs.pfnHookUserMsg("Damage", sDamageReceive);

	s_health = 0.0f;
	s_accuracy[0] = 0.0f;
	s_accuracy[1] = 0.0f;
}

void Ic::MessagesSetAccuracy(Side side, float a)
{
	s_accuracy[static_cast<int>(side)] = a;
}


bool Ic::IsPlayerDead()
{
	return (s_health <= 0) ? true : false;
}

int Ic::PlayerHealth()
{
	return s_health;
}

int Ic::PlayerMaxHealth()
{
	return 100; // TODO
}

float Ic::PlayerAccuracy(Side side)
{
	return s_accuracy[static_cast<int>(side)];
}
