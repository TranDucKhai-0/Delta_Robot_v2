#include "task_motor_control.h"


void Robot_Motor_Control_Task(void *pvParameters){
    // Đưa cánh tay về Home


    // Cho phép các task bị dừng trước đó hoạt động
    if (g_handle_planner != NULL) {
        vTaskResume(g_handle_planner);
    }
    
    if (g_handle_kinematics != NULL) {
        vTaskResume(g_handle_kinematics);
    }

    // luồng chạy chính của task điều khiển động cơ
    theta_t theta_target;
    while (1) {
        // Kiểm tra có bộ góc mục tiêu mới nào được gửi từ kinematics đến control hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY)) {
            
        }
    }
}