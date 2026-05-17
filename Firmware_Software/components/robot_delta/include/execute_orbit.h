#pragma once

#include "globals.h"
#include "type_data.h"
#include "robot_delta.h"


void Execute_Linear_Motion(robot_object_t *p_robot, point_t p_start, point_t p_end, float target_speed, velocity_profile_t profile);
void Execute_Bezier_Motion(robot_object_t *p_robot, point_t p_A, point_t p_C, point_t p_B, float target_speed, velocity_profile_t profile);