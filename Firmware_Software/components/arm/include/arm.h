#pragma once

#include <stdint.h>
#include "driver/ledc.h" // Thêm thư viện băm xung bằng phần cứng của ESP-IDF

typedef struct arm{
    uint8_t _pin; // chân GPIO điều khiển động cơ
    ledc_channel_t _ledc_channel; // kênh LEDC để xuất xung PWM
} arm_object_t;

void Arm_Init(arm_object_t *arm, uint8_t pin);