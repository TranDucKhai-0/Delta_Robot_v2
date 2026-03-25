#pragma once

#include "globals.h"

typedef struct robot_delta{
    // parameters of the robot
    const float A, RF, RE; // mm
    // vùng hoạt động hiệu quả
    const float Z_MIN, Z_MAX, R2; // R2 là bình phương bán kính hoạt động

    point_t _end_effector_current; // mm
    theta_t _theta_current; // deg

    point_t _end_effector_target; // mm
    theta_t _theta_target; // deg

    bool _has_end_effector_changed; // end-effector 
    bool _has_theta_changed; // theta

} robot_object_t;

robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2, const uint8_t PIN_ARM_1, const uint8_t PIN_ARM_2, const uint8_t PIN_ARM_3);

void Robot_Call_Kinematics_Inverse(robot_object_t* p_robot, point_t *p_point_target);
void Robot_Call_Kinematics_Forward(robot_object_t* p_robot, theta_t *p_theta_target);
