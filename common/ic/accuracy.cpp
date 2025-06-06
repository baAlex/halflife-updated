/*

Copyright (c) 2025 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "accuracy.hpp"
#include "base.hpp"

#include <math.h>
#include <string.h>


void Ic::Accuracy::Initialise()
{
	m_set = false;

	m_prev_origin = {};
	m_prev_angles = {};

	m_walk_speed = 0.0f;
	m_look_speed = 0.0f;
	m_prev_look_speed = 0.0f;
}


static constexpr float WALK_SMOOTH = 10.0f;            // Origin has a quite low precision, we need to smooth it
static constexpr float LOOK_SMOOTH[2] = {12.0f, 3.0f}; // Not here

static constexpr float WALK_CONTRIBUTION = 1.0f;
static constexpr float LOOK_CONTRIBUTION = 0.5f;


float Ic::Accuracy::Sample(Ic::Vector2 origin, Ic::Vector2 angles, float max_speed, float dt)
{
	if (m_set == false)
	{
		m_set = true;

		m_prev_origin = origin;
		m_prev_angles = angles;

		m_walk_speed = 0.0f;
		m_look_speed = 0.0f;
		m_prev_look_speed = 0.0f;

		return 0.0f;
	}

	if (dt <= 0.001f) // Things happen
		return 0.0f;

	// Walk
	{
		const Ic::Vector2 delta =
		    Scale(Subtract(origin, m_prev_origin), 1.0f / (dt / WALK_CONTRIBUTION)); // We want deltas in game units
		m_prev_origin = origin;

		m_walk_speed = Ic::HolmerMix(Length(delta) / max_speed, m_walk_speed, WALK_SMOOTH, dt);
	}

	// Look
	{
		const float dx = Ic::AnglesDifference(angles[0], m_prev_angles[0]) / dt;
		const float dz = Ic::AnglesDifference(angles[1], m_prev_angles[1]) / dt;
		m_prev_angles = angles;

		const float speed = sqrtf(dx * dx + dz * dz) / (360.0f / LOOK_CONTRIBUTION);
		m_look_speed =
		    Ic::AnglesHolmerMix(speed, m_look_speed, (speed > m_prev_look_speed) ? LOOK_SMOOTH[0] : LOOK_SMOOTH[1], dt);
		m_prev_look_speed = m_look_speed;
	}

	return Get();
}


float Ic::Accuracy::Get() const
{
	return m_walk_speed + m_look_speed;
}
