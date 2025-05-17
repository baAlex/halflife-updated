/*

Copyright (c) 2025 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#ifndef IC_BASE_HPP
#define IC_BASE_HPP

namespace Ic
{

float DegToRad(float deg);
float RadToDeg(float rad);
float Min(float a, float b);
float Max(float a, float b);
float Clamp(float x, float min, float max);
float Mix(float a, float b, float f);
float FmodFloored(float x, float y);
float AnglesDifference(float a, float b);
float AnglesMix(float a, float b, float f);
float AnglesHolmerMix(float a, float b, float d, float dt);
float HolmerMix(float a, float b, float d, float dt);
float ClampAroundCentre(float x, float centre, float range);

} // namespace Ic

#endif
