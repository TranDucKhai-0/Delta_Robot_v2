#include "task_kinematics.h"

#include "kinematics.h"
#include "math_until.h"


// ==============================Task Tính Toán Động Học Ngược===============================
void Robot_Kinematics_Task(void *pvParameters){
    point_t point_target;
    theta_t theta_target; // chứa kết quả tính toán góc theta mục tiêu từ điểm mục tiêu
    while (1) {
        // Kiểm tra có điểm mục tiêu mới nào được gửi từ planner đến kinematics hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY)) {
            
            theta_target = Kinematics_Call_Inverse(&g_p_robot, &point_target); // Tính toán góc theta mục tiêu từ điểm mục tiêu và cập nhật vào struct robot
            
            xQueueSend(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY); // Gửi góc theta mục tiêu đã tính toán được về planner để planner có thể sử dụng trong quá trình lập kế hoạch di chuyển.

            xSemaphoreTake(g_p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
            g_p_robot->end_effector_current = point_target; //Cập nhật điểm hiện tại vào Robot
            g_p_robot->has_end_effector_current_changed = true; // Đặt cờ để báo hiệu rằng điểm hiện tại đã thay đổi
            g_p_robot->theta_current = theta_target; //Cập nhật điểm hiện tại vào Robot
            g_p_robot->has_theta_current_changed = true; // Đặt cờ để báo hiệu rằng góc hiện tại đã thay đổi
            xSemaphoreGive(g_p_robot->lock); // Unlock sau khi đã đọc điểm hiện tại
        }
    }
}