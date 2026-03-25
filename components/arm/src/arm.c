#include "arm.h"

static void _Arm_Ledc_Channel_Init(arm_object_t *arm) {
    static ledc_channel_t _next_channel = LEDC_CHANNEL_0;
    arm->_ledc_channel = _next_channel;
    _next_channel = (ledc_channel_t)(_next_channel + 1);

    // Cấu hình Kênh xuất xung ra chân GPIO 
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode     = LEDC_HIGH_SPEED_MODE;
    ledc_channel.channel        = arm->_ledc_channel;
    ledc_channel.timer_sel      = LEDC_TIMER_0;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = arm->_pin;
    ledc_channel.duty           = 0; // Mới bật lên thì chưa xuất xung
    ledc_channel.hpoint         = 0;
    ledc_channel_config(&ledc_channel);
}

void Arm_Init(arm_object_t *arm, uint8_t pin) {
    // Gán chân GPIO cho arm
    arm->_pin = pin;
    
    // Cấu hình kênh LEDC để xuất xung PWM
    _Arm_Ledc_Channel_Init(arm);
}