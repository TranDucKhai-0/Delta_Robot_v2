#include "task_planner.h"

void Robot_Planner_Task(void *pvParameters){
    point_t point_target;
    while (1) {
        // Kiểm tra có điểm mục tiêu mới nào được gửi từ giao diện người dùng đến planner hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_udp_to_planner, &point_target, portMAX_DELAY)) {
            
        }
    }
}