#pragma once

#include "freertos/queue.h"
#include <stdint.h>

// Khởi tạo GPIO cho arm
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