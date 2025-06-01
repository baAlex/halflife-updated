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
#include "cl_dll.h"


struct Sample
{
	float x;
	float y;
	float z;
	double time;
};

constexpr int SAMPLES_NO = 8; // TODO

static Sample s_samples[SAMPLES_NO];
static Sample* s_cursor = s_samples;
static double s_next_sampling;
static float s_speed;


void Ic::AccuracyInitialise()
{
	// gEngfuncs.Con_Printf("AccuracyInitialise()\n");

	memset(s_samples, 0, sizeof(Sample) * SAMPLES_NO);
	s_cursor = s_samples;
	s_next_sampling = 0.0;
	s_speed = 0.0f;
}


void Ic::AccuracySample(float x, float y, float z, float max_speed, double time)
{
	// On the client, we are called directly by the engine, every frame:
	//    Ic::AccuracySample <- V_CalcRefdef()

	// It has to be done there as it happens after client prediction, right before
	// rendering. Other places that seems more sensical at glance like
	// HUD_PlayerMove() where prediction happens, or HUD_ProcessPlayerState() that
	// receives server data; have its own problems, first one doesn't run on demos,
	// last one runs at a lower frequency.

	// On the server, a TODO for now

	if (time < s_next_sampling)
		return;

	// Add new sample
	{
		s_next_sampling = time + 1.0 / 30.0; // TODO

		s_cursor += 1;
		if (s_cursor == s_samples + SAMPLES_NO)
			s_cursor = s_samples;

		s_cursor->x = x;
		s_cursor->y = y;
		s_cursor->z = z;
		s_cursor->time = time;
	}

	// Calculate speed
	{
		Sample* c = s_cursor;
		double total_distance = 0.0;
		double total_time = 0.0;

		for (int i = 0; i < SAMPLES_NO - 1; i += 1)
		{
			Sample* p = c - 1;
			if (p < s_samples)
				p = s_samples + SAMPLES_NO - 1;

			const float dx = c->x - p->x;
			const float dy = c->y - p->y;
			const float dz = c->z - p->z;
			const double dt = c->time - p->time;

			total_distance += sqrtf(dx * dx + dy * dy + dz * dz);
			total_time += dt;

			c = p;
		}

		s_speed = static_cast<float>(total_distance / total_time) / max_speed;
	}
}

float Ic::Accuracy()
{
	// gEngfuncs.Con_Printf("%f\n", s_speed);
	return s_speed;
}
