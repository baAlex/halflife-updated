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

// ORDER OF INCLUDES IS THIS AND NO OTHER
#include "vector.h"
#include "Platform.h"
#include "pm_movevars.h"
#include "usercmd.h"
#include "ref_params.h"

#ifndef IC_VIEW_HPP
#define IC_VIEW_HPP

namespace Ic
{

void ViewInitialise();
void ViewUpdate(struct ref_params_s* input_output);

void ViewApplyPunch(int axis, float punch);
void ViewGetChasePosition(int target, const float* cl_angles, float* out_origin, float* out_angles);

} // namespace Ic

#endif
