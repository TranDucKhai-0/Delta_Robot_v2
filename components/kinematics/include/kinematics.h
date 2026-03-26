#pragma once

#include "globals.h"
#include "type_data.h"
#include "robot_delta.h"



// Gọi Động học nghịch trả về gốc mục tiêu
theta_t Kinematics_Call_Inverse(robot_object_t* p_robot, point_t *p_point_target);


// Gọi Động học thuận
point_t Kinematics_Call_Forward(robot_object_t* p_robot, theta_t *p_theta_target);
