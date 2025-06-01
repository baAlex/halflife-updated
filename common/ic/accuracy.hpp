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

class Accuracy
{
  public:
	void Initialise();
	float Sample(float x, float y, float z, float max_speed, float time); // Time in seconds
	float Get() const;

  private:
	struct Sample_
	{
		float x;
		float y;
		float z;
		float time;
	};

	static const constexpr int SAMPLES_NO = 8; // TODO

	Sample_ m_samples[SAMPLES_NO];
	int m_cursor;
	int m_total_samples;

	float m_speed;
};

} // namespace Ic

#endif
