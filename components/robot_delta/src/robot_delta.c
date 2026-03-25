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
        ._has_end_effector_changed = false,
        ._has_theta_changed = false
    };
    return robot;
}

static bool _Robot_Is_In_Workspace(const robot_object_t* p_robot, point_t *p_point){
    // kiểm tra độ cao z
    if (p_point->z > p_robot->Z_MAX || p_point->z < p_robot->Z_MIN)
        return false;

    // kiểm tra bán kính có bé hơn R
    if (sqr(p_point->x) + sqr(p_point->y) > p_robot->R2)
        return false;

    return true;
}

// Hàm này sẽ gọi hàm kinematics_inverse để tính toán góc theta từ điểm mục tiêu, và cập nhật trạng thái của robot.
static void _Robot_Call_Kinematics_Inverse(robot_object_t* p_robot, point_t *p_point_target){
    if(p_robot->_has_end_effector_changed) { 
        // Nếu điểm mục tiêu không nằm trong vùng hoạt động hiệu quả, nó sẽ đặt cờ _has_end_effector_changed thành false để báo hiệu rằng điểm đó không hợp lệ.
        if (!_Robot_Is_In_Workspace(p_robot, p_point_target)){
            p_robot->_has_end_effector_changed = false;
            return;
        }

        if(Calculate_Kinematics_Inverse(p_robot, p_point_target, &(p_robot->_theta_current))){
            p_robot->_has_theta_changed = true; // Đặt cờ này thành true để báo hiệu rằng góc theta đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }

        p_robot->_has_end_effector_changed = false; // Đặt cờ này thành false sau khi đã dùng tọa độ hiện tại, để tránh tính toán lại nếu điểm mục tiêu không thay đổi.
    }
}

// Hàm này sẽ gọi hàm kinematics_forward để tính toán vị trí của end-effector từ góc theta mục tiêu
static void _Robot_Call_Kinematics_Forward(robot_object_t* p_robot, theta_t *p_theta_target){
    if(_has_theta_changed) {
        if(Calculate_Kinematics_Forward(p_robot, p_theta_target, &(p_robot->_end_effector_current))){
            p_robot->_has_end_effector_changed = true; // Đặt cờ này thành true để báo hiệu rằng vị trí end-effector đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }
        p_robot->_has_theta_changed = false; // Đặt cờ này thành false sau khi đã dùng bộ gốc hiện tại, để tránh tính toán lại nếu góc theta không thay đổi.
    }
}