#include "task_planner.h"

#include "type_data.h"
#include "globals.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MODE_HOMING 0
#define MODE_AUTOMATIC 1
#define MODE_MANUAL 2
#define MODE_PICK_AND_PLACE 3



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