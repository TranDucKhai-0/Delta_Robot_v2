#pragma once

#include "globals.h"
#include "type_data.h"
#include "robot_delta.h"

// Động học nghịch
bool Calculate_Kinematics_Inverse(const robot_object_t *self, point_t *p_point_target, theta_t *p_theta_target);

// Động học thuận
bool Calculate_Kinematics_Forward(const robot_object_t *self, theta_t *p_theta_target, point_t *p_point_target);