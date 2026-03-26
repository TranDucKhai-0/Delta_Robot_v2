#include "task_planner.h"

#include "type_data.h"
#include "globals.h"
#include "robot_delta.h"
#include "kinematics.h"
#include "math_until.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MODE_HOMING 0
#define MODE_AUTOMATIC 1
#define MODE_MANUAL 2
#define MODE_PICK_AND_PLACE 3

static void _Robot_Homing(robot_object_t *p_robot) {
    uint8_t homing_step = 50;
    theta_t theta_home = {0.0f, 0.0f, 0.0f}; // Góc theta home (có thể điều chỉnh tùy theo cấu hình robot)

    point_t point_home = Kinematics_Call_Forward(p_robot, &theta_home); // Tính toán điểm home từ góc theta home
    point_t point_target; // chứa kết quả nội suy

    for (uint8_t i = 0; i < homing_step; i++) {
        // Kiểm tra cờ break homing trước khi tiếp tục nội suy
        if(p_robot->should_break_homing) {
            p_robot->should_break_homing = false; // Reset cờ sau khi đã dùng
            return; // Thoát khỏi quá trình homing nếu cờ break được đặt
        }
        
        // Nội suy tuyến tính từ theta_current về theta_home
        point_target = Math_Linear_Interpolation(&p_robot->end_effector_current, &point_home, (float)i / homing_step);

        // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
        xQueueSend(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY);

        // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}



// =============================Task Planner =============================
void Robot_Planner_Task(void *pvParameters){
    point_t point_target = {0.0f, 0.0f, 0.0f, 0}; // chứa điểm mới từ PC
    point_t point_current; // chứa điểm trước đó

    while (1) {
        //Kiểm tra xem có tọa độ mới từ máy tính không (Không có thì lấy điểm cũ)
        xQueueReceive(g_queue_udp_to_planner, &point_target, portMAX_DELAY);
        
        point_current.mode = point_target.mode; // Cập nhật mode mới

        if(point_current.mode == MODE_HOMING) {
            
        } 
        else if(point_current.mode == MODE_AUTOMATIC){

        } 
        else if(point_current.mode == MODE_MANUAL){

        } 
        else if(point_current.mode == MODE_PICK_AND_PLACE){

        }

        //Ngừng 20ms để cho task khác trong Core0 hoạt động
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}