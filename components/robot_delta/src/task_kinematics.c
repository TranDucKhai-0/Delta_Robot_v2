#include "task_kinematics.h"

#include "kinematics.h"
#include "math_until.h"

// Hàm này sẽ kiểm tra xem điểm mục tiêu có nằm trong vùng hoạt động hiệu quả của robot hay không.
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
    if(p_robot->_has_end_effector_target_changed) { 
        // Nếu điểm mục tiêu không nằm trong vùng hoạt động hiệu quả, nó sẽ đặt cờ _has_end_effector_target_changed thành false để báo hiệu rằng điểm đó không hợp lệ.
        if (!_Robot_Is_In_Workspace(p_robot, p_point_target)){
            p_robot->_has_end_effector_target_changed = false;
            return;
        }

        if(Calculate_Kinematics_Inverse(p_robot, p_point_target, &(p_robot->_theta_current))){
            p_robot->_has_theta_current_changed = true; // Đặt cờ này thành true để báo hiệu rằng góc theta đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }

        p_robot->_has_end_effector_target_changed = false; // Đặt cờ này thành false sau khi đã dùng tọa độ mục tiêu, để tránh tính toán lại nếu điểm mục tiêu không thay đổi.
    }
}

// Hàm này sẽ gọi hàm kinematics_forward để tính toán vị trí của end-effector từ góc theta mục tiêu
static void _Robot_Call_Kinematics_Forward(robot_object_t* p_robot, theta_t *p_theta_target){
    if(p_robot->_has_theta_target_changed) {
        if(Calculate_Kinematics_Forward(p_robot, p_theta_target, &(p_robot->_end_effector_current))){
            p_robot->_has_end_effector_current_changed = true; // Đặt cờ này thành true để báo hiệu rằng vị trí end-effector đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }
        p_robot->_has_theta_target_changed = false; // Đặt cờ này thành false sau khi đã dùng bộ gốc mục tiêu, để tránh tính toán lại nếu góc theta không thay đổi.
    }
}

// ==============================Task===============================
void Robot_Kinematics_Task(void *pvParameters){
    point_t point_target;
    while (1) {
        // Kiểm tra có điểm mục tiêu mới nào được gửi từ planner đến kinematics hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY)) {
            
        }
    }
}