#include "task_motor_control.h"


void Robot_Motor_Control_Task(void *pvParameters){
    theta_t theta_target;
    while (1) {
        // Kiểm tra có bộ góc mục tiêu mới nào được gửi từ kinematics đến control hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY)) {
            
        }
    }
}