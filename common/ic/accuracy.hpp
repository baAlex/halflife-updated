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
	float Sample(float origin_x, float origin_y, float angle_x, float angle_z, float max_speed, float dt);
	float Get() const;

  private:
	bool m_set;

	float m_prev_origin_x;
	float m_prev_origin_y;
	float m_prev_angle_x;
	float m_prev_angle_z;

	float m_walk_speed;
	float m_look_speed;
	float m_prev_look_speed;
};

} // namespace Ic

#endif
