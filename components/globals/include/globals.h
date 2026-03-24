#pragma once

#include "freertos/queue.h"
#include <stdio.h>

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

    // Bổ sung toán tử != 
    bool operator!=(const target_point_t& other) const {
        return (x != other.x || y != other.y || z != other.z || mode != other.mode);
    }
} target_point_t;

typedef struct {
    float arm_1;
    float arm_2;
    float arm_3;
} current_theta_t;