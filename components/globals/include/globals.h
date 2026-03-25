#pragma once

#include "robot_delta.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

// Gán chân GPIO cho arm
#define ARM_1 26
#define ARM_2 25
#define ARM_3 33 

// Khai báo biến toàn cục chứa trạng thái của robot
extern robot_object_t *g_p_robot; 

// Queue 1: Truyền tọa độ thô từ UDP sang thuật toán làm mượt
extern QueueHandle_t g_queue_udp_to_planner;  

// Queue 2: Truyền tọa độ đã làm mượt sang Core 1 tính IK
extern QueueHandle_t g_queue_planner_to_core1;