#include "robot_delta.h"

static const theta_t _PHI = {0.0f, 2.094f, -2.094f}; // rad

robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2, const uint8_t PIN_ARM_1, const uint8_t PIN_ARM_2, const uint8_t PIN_ARM_3){
    robot_object_t robot = {
        .A = A,
        .RF = RF,
        .RE = RE,
        .Z_MIN = Z_MIN,
        .Z_MAX = Z_MAX,
        .R2 = R2,
        ._is_flat_end_effector = false,
        ._is_flat_theta = false
    };
    return robot;
}