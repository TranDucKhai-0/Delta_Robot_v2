#pragma once

#include "globals.h"

typedef struct robot_delta{
    // parameters of the robot
    const float A, RF, RE; // mm
    // vùng hoạt động hiệu quả
    const float Z_MIN, Z_MAX, R2; // R2 là bình phương bán kính hoạt động

    point_t _end_effector_current; // mm
    theta_t _theta_current; // deg

    bool _is_flat_end_effector; // end-effector
    bool _is_flat_theta; // theta

} robot_object_t;

robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2, const uint8_t PIN_ARM_1, const uint8_t PIN_ARM_2, const uint8_t PIN_ARM_3);

