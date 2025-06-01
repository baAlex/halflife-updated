/*

Copyright (c) 2025 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#ifndef IC_ACCURACY_HPP
#define IC_ACCURACY_HPP

namespace Ic
{

void AccuracyInitialise();
void AccuracySample(float x, float y, float z, float max_speed, double time); // Time in seconds
float Accuracy();

} // namespace Ic

#endif
