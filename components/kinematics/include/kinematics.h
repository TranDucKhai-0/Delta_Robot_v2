#pragma once

#include "globals.h"
#include "robot_delta.h"

// Động học nghịch
bool Calculate_Inverse_Kinematics(const robot_object_t *self, point_t *p_point_current, theta_t *p_theta_target);

// Động học thuận
bool Calculate_Forward_Kinematics(const robot_object_t *self, theta_t *p_theta_current, point_t *p_point_target);