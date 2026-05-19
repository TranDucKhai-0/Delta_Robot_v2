#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>

// MODE
#define MODE_HOMING 0
#define MODE_AUTOMATIC 1
#define MODE_MANUAL 2
#define MODE_PICK_AND_PLACE 3

typedef struct robot_delta robot_object_t;

// Gán chân GPIO cho arm
#define ARM_1 26
#define ARM_2 25
#define ARM_3 33 

#define GRIPPER 2   // Chân GPIO cho gripper (điều khiển đóng/mở) (tạm test bằng chân LED)
 
#define GRIPPER_TIME_DELAY_MS 1000 // Thời gian delay để đảm bảo gripper đã kịp gắp/thả được vật trước khi tiếp tục thực hiện các bước tiếp theo (ms)

#define CYCLE_TIME_MS 20.0f // Chu kỳ thời gian cho mỗi bước di chuyển (ms)

#define SPEED_MM_PER_SEC 500.0f // Tốc độ di chuyển max của robot (mm/s)

#define GRIPPER_DEFAULT_STATE false // Trạng thái nhả mặc định của gripper khi khởi động là false

// Khai báo biến toàn cục chứa trạng thái của robot
extern robot_object_t *g_p_robot; 

// Queue 1: Truyền tọa độ thô từ UDP sang task lập kế hoạch (Planner) để làm mượt
extern QueueHandle_t g_queue_udp_to_planner;  

// Queue 2: Truyền tọa độ đã làm mượt sang task tính kinematics
extern QueueHandle_t g_queue_planner_to_kinematics;

// Queue 3: Truyền góc theta đã tính toán từ thuật toán IK sang task điều khiển động cơ
extern QueueHandle_t g_queue_kinematics_to_control;

// 
extern TaskHandle_t g_handle_planner;
extern TaskHandle_t g_handle_kinematics;