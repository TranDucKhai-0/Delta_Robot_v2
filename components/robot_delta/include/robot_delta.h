#pragma once

#include "type_data.h"

#include "task_kinematics.h"
#include "task_motor_control.h"
#include "task_planner.h"

#include <stdbool.h>

typedef struct robot_delta{
    // parameters of the robot
    float A, RF, RE; // mm
    // vùng hoạt động hiệu quả
    float Z_MIN, Z_MAX, R2; // R2 là bình phương bán kính hoạt động

    // trạng thái hiện tại của robot (điều khiển động cơ bằng góc theta này)
    point_t _end_effector_current; // mm
    theta_t _theta_current; // deg

    bool _has_end_effector_current_changed; // end-effector 
    bool _has_theta_current_changed; // theta

    // trạng thái mục tiêu của robot (lưu trữ điểm mục tiêu và góc theta mục tiêu, tính toán động học bằng các biến này)
    point_t _end_effector_target; // mm
    theta_t _theta_target; // deg

    bool _has_end_effector_target_changed; // end-effector 
    bool _has_theta_target_changed; // theta

} robot_object_t;

robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2, const uint8_t PIN_ARM_1, const uint8_t PIN_ARM_2, const uint8_t PIN_ARM_3);
