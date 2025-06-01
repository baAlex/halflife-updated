/*

Copyright (c) 2025 Alexander Brandt

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at https://mozilla.org/MPL/2.0/.

This Source Code Form is "Incompatible With Secondary Licenses", as
defined by the Mozilla Public License, v. 2.0.
*/

#include "accuracy.hpp"

#include <math.h>
#include <string.h>


void Ic::Accuracy::Initialise()
{
	memset(m_samples, 0, sizeof(Sample) * SAMPLES_NO);
	m_cursor = 0;
	m_total_samples = 0;

	m_speed = 0.0f;
}


float Ic::Accuracy::Sample(float x, float y, float z, float max_speed, float time)
{
	// On the client, we are called directly by the engine, every frame:
	//    Ic::AccuracySample <- V_CalcRefdef()

	// It has to be done there as it happens after client prediction, right before
	// rendering. Other places that seems more sensical at glance like
	// HUD_PlayerMove() where prediction happens, or HUD_ProcessPlayerState() that
	// receives server data; have its own problems, first one doesn't run on demos,
	// last one runs at a lower frequency.

	// On the server, a TODO for now

	// Add new sample
	{
		m_cursor = (m_cursor + 1 < SAMPLES_NO) ? m_cursor + 1 : 0;

		m_samples[m_cursor].x = x;
		m_samples[m_cursor].y = y;
		m_samples[m_cursor].z = z;
		m_samples[m_cursor].time = time;
	}

	// Do we have enough?
	m_total_samples += 1;
	if (m_total_samples < SAMPLES_NO) // Let's pray the branch prediction gods
		return;

	// Calculate speed
	{
		const Sample_* c = m_samples + m_cursor;
		double total_distance = 0.0;
		double total_time = 0.0;

		for (int i = 0; i < SAMPLES_NO - 1; i += 1)
		{
			const Sample_* p = c - 1;
			if (p < m_samples)
				p = m_samples + SAMPLES_NO - 1;

			const float dx = c->x - p->x;
			const float dy = c->y - p->y;
			const float dz = c->z - p->z;
			const double dt = c->time - p->time;

			total_distance += sqrtf(dx * dx + dy * dy + dz * dz);
			total_time += dt;

			c = p;
		}

		m_speed = static_cast<float>(total_distance / total_time) / max_speed;
	}

	return Get();
}


float Ic::Accuracy::Get() const
{
	return m_speed;
}
