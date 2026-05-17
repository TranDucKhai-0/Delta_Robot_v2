#pragma once

#include <stdint.h>

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

typedef enum {
    PROFILE_ACCEL = 0, // Tăng tốc (Khởi hành: 0 -> Vmax)
    PROFILE_CRUISE,    // Chạy đều (Lướt cua: Vmax không đổi)
    PROFILE_DECEL,     // Giảm tốc (Chuẩn bị dừng: Vmax -> 0)
    PROFILE_S_CURVE    // Đầy đủ (Dừng -> Dừng: Dùng cho đoạn đơn lẻ)
} velocity_profile_t;

typedef struct {
    // ----------------------------------------------------
    // CẤU TRÚC 1: Dùng cho Mode 0, 1, 2 
    // ----------------------------------------------------
    point_t target; 

    // ----------------------------------------------------
    // CẤU TRÚC 2: Dùng riêng cho Mode 3 (Pick & Place)
    // ----------------------------------------------------
    point_t pick;   // Tọa độ gắp
    point_t place;  // Tọa độ thả
} udp_payload_t;