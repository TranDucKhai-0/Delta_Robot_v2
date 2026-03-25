#include "robot_delta.h"
#include "kinematics.h"
#include "math_utils.h"


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

static bool _Robot_Is_In_Workspace(const robot_object_t* robot, point_t *point){
    // kiểm tra độ cao z
    if (point->z > robot->Z_MAX || point->z < robot->Z_MIN)
        return false;

    // kiểm tra bán kính có bé hơn R
    if (sqr(point->x) + sqr(point->y) > robot->R2)
        return false;

    return true;
}