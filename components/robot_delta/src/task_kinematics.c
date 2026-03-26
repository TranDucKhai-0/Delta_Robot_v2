#include "task_kinematics.h"

#include "kinematics.h"
#include "math_until.h"


// ==============================Task===============================
void Robot_Kinematics_Task(void *pvParameters){
    point_t point_target;
    while (1) {
        // Kiểm tra có điểm mục tiêu mới nào được gửi từ planner đến kinematics hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY)) {
            
        }
    }
}