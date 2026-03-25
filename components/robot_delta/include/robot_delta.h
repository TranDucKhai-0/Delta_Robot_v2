#pragma once

#include "globals.h"

typedef struct robot_delta{
    // parameters of the robot
    const float A, RF, RE; // mm
    // vùng hoạt động hiệu quả
    const float Z_MIN, Z_MAX, R2; // R2 là bình phương bán kính hoạt động

    // trạng thái hiện tại của robot (điều khiển động cơ bằng góc theta này)
    point_t _end_effector_current; // mm
    theta_t _theta_current; // deg

    // trạng thái mục tiêu của robot (lưu trữ điểm mục tiêu và góc theta mục tiêu, tính toán động học bằng các biến này)
    point_t _end_effector_target; // mm
    theta_t _theta_target; // deg

    bool _has_end_effector_changed; // end-effector 
    bool _has_theta_changed; // theta

} robot_object_t;

robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2, const uint8_t PIN_ARM_1, const uint8_t PIN_ARM_2, const uint8_t PIN_ARM_3);

// Hàm chạy trong 1 task chuyên tính toán động học
void Robot_Kinematics_Task(void *pvParameters);

// Hàm chạy trong 1 task chuyên xuất xung điều khiển động cơ
void Robot_Motor_Control_Task(void *pvParameters);