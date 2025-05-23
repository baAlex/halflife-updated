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

#include "fog.hpp"

#include "cl_dll.h"
#include "SDL2/SDL_video.h"
#include "triangleapi.h"
#include "cvardef.h"


#ifndef GLAPI
#if defined(GLAD_GLAPI_EXPORT)
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(GLAD_GLAPI_EXPORT_BUILD)
#if defined(__GNUC__)
#define GLAPI __attribute__((dllexport)) extern
#else
#define GLAPI __declspec(dllexport) extern
#endif
#else
#if defined(__GNUC__)
#define GLAPI __attribute__((dllimport)) extern
#else
#define GLAPI __declspec(dllimport) extern
#endif
#endif
#elif defined(__GNUC__) && defined(GLAD_GLAPI_EXPORT_BUILD)
#define GLAPI __attribute__((visibility("default"))) extern
#else
#define GLAPI extern
#endif
#else
#define GLAPI extern
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY*
#endif
#endif


// Mini Glad implementation

typedef int GLint;
typedef float GLfloat;
typedef unsigned int GLenum;

typedef void(APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void(APIENTRYP PFNGLFOGIPROC)(GLenum pname, GLint param);
typedef void(APIENTRYP PFNGLFOGFPROC)(GLenum pname, GLfloat param);
typedef void(APIENTRYP PFNGLFOGFVPROC)(GLenum pname, const GLfloat* params);

GLAPI PFNGLENABLEPROC glad_glEnable = NULL;
GLAPI PFNGLFOGIPROC glad_glFogi = NULL;
GLAPI PFNGLFOGFPROC glad_glFogf = NULL;
GLAPI PFNGLFOGFVPROC glad_glFogfv = NULL;

#define GL_FOG 0x0B60
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_MODE 0x0B65
#define GL_FOG_COLOR 0x0B66
#define GL_LINEAR 0x2601
#define GL_EXP 0x0800
#define GL_EXP2 0x0801

// ----


static int s_fog_available = 0;
static cvar_t* s_color_cvar;
static cvar_t* s_density_cvar;

static float s_color[4] = {86.0f / 255.0f, 91.0f / 255.0f, 104.0f / 255.0f, 1.0f};
static float s_density = 1.0f / 3000.0f;


static void sUpdateCallback()
{
	const char* color = gEngfuncs.pfnGetCvarString("fog_color");
	if (color != nullptr)
	{
		int temp[4] = {};
		sscanf(color, "%i %i %i", &temp[0], &temp[1], &temp[2]);

		s_color[0] = static_cast<float>(temp[0]) / 255.0f;
		s_color[1] = static_cast<float>(temp[1]) / 255.0f;
		s_color[2] = static_cast<float>(temp[2]) / 255.0f;
	}

	const float density = gEngfuncs.pfnGetCvarFloat("fog_density");
	if (density != 0.0f)
		s_density = 1.0f / density;
}


void Ic::FogVideoInitialise()
{
	// «Called when the game initializes and whenever the vid_mode is changed»
	// Valve (cl_dll/cdll_int.cpp)

	// We are called directly by the engine:
	//    Ic::FogVideoInitialise() <- HUD_VidInit()

	// ----

	// Dynamical load OpenGl functions,
	// using SDL to make it portable (the un-portable alternative is
	// to include an ancient Gl header "opengl.h" and link against
	// an equally ancient library. In our case is up to SDL to provide
	// a proper Gl implementation)
	glad_glEnable = (PFNGLENABLEPROC)(SDL_GL_GetProcAddress("glEnable"));
	glad_glFogi = (PFNGLFOGIPROC)(SDL_GL_GetProcAddress("glFogi"));
	glad_glFogf = (PFNGLFOGFPROC)(SDL_GL_GetProcAddress("glFogf"));
	glad_glFogfv = (PFNGLFOGFVPROC)(SDL_GL_GetProcAddress("glFogfv"));

	if (glad_glEnable == nullptr || glad_glFogi == nullptr || //
	    glad_glFogf == nullptr || glad_glFogfv == nullptr)
	{
		gEngfuncs.Con_Printf("Error at loading OpenGl functions.\n");
	}
	else
	{
		s_fog_available = 1;
	}

	// Register variables
	s_color_cvar = gEngfuncs.pfnRegisterVariable("fog_color", "86 91 104", FCVAR_SPONLY);
	s_density_cvar = gEngfuncs.pfnRegisterVariable("fog_density", "3000", FCVAR_SPONLY);

	gEngfuncs.pfnAddCommand("fog_update", sUpdateCallback);
}


void Ic::FogDraw()
{
	if (s_fog_available == 0)
		return;

	const float s = 0.01f;  // Start and end doesn't affect EXP2 mode, which in
	const float e = 500.0f; // turn is hardcoded because is the best looking one

	glad_glEnable(GL_FOG);
	glad_glFogfv(GL_FOG_COLOR, s_color);
	glad_glFogi(GL_FOG_MODE, GL_EXP2);
	glad_glFogf(GL_FOG_DENSITY, s_density);
	glad_glFogf(GL_FOG_START, s);
	glad_glFogf(GL_FOG_END, e);

	gEngfuncs.pTriAPI->FogParams(s_density, 0);
	gEngfuncs.pTriAPI->Fog(s_color, s, e, 1);
}
