#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

// Gán chân GPIO cho arm
#define ARM_1 26
#define ARM_2 25
#define ARM_3 33 

// Cốt lõi chỉ có tọa độ 3D (x, y, z) để tính toán và điều khiển động cơ
typedef struct {
    float x;
    float y;
    float z;
    uint8_t mode;
} point_t;

typedef struct {
    float arm_1;
    float arm_2;
    float arm_3;
} theta_t;

// Queue 1: Truyền tọa độ thô từ UDP sang thuật toán làm mượt
extern QueueHandle_t g_queue_udp_to_planner;  

// Queue 2: Truyền tọa độ đã làm mượt sang Core 1 tính IK
extern QueueHandle_t g_queue_planner_to_core1;