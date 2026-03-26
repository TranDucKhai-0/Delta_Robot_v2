#include "task_motor_control.h"

#include "arm.h"
#include "globals.h"
#include "type_data.h"
#include "arm.h"
#include "robot_delta.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>


// Các thông số ép xung cho MG90S (Độ phân giải 14-bit)
// 14-bit có nghĩa là 1 chu kỳ 20ms được chia làm 16384 nấc
#define DUTY_MIN 410  // Tương đương xung 0.5ms (Góc 0 độ)
#define DUTY_MAX 1966 // Tương đương xung 2.0ms (Góc 180 độ)


static void _Robot_Timer_Init() {
    // Tần số 50Hz, Độ phân giải 14-bit
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode       = LEDC_HIGH_SPEED_MODE;
    ledc_timer.timer_num        = LEDC_TIMER_0; // 3 Arm xài chung 1 Timer
    ledc_timer.duty_resolution  = LEDC_TIMER_14_BIT;
    ledc_timer.freq_hz          = 50;           // Tần số chuẩn của MG996R (50Hz)
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);
}

// Hàm tính góc đầu ra cho 1 cánh tay
static float _Robot_Calculate_Servo_Angle(float angle_rad) {
    // Đổi về Độ (Degrees)
    float angle_deg = angle_rad * 180.0f / M_PI;

    // Bù góc cơ khí (Đưa góc 0 của IK về tâm 90 của Servo)
    float servo_angle = 90.0f - angle_deg;

    // Chặn an toàn cơ khí (Chống gãy nhông Servo)
    if (servo_angle < 0.0f) return 0.0f;
    if (servo_angle > 180.0f) return 180.0f;
    
    return servo_angle;
}

// ánh xạ Xung (Degree -> Duty)
static uint32_t _Robot_Calculate_Duty(float servo_angle) {
    // Ánh xạ tuyến tính từ dải góc (0 - 180) sang dải xung (410 - 1966)
    return DUTY_MIN + (uint32_t)((servo_angle / 180.0f) * (DUTY_MAX - DUTY_MIN));
}

// Giao tiếp phần cứng (Duty -> LEDC)
static void _Robot_Set_PWWM_Duty(arm_object_t *arm, uint32_t duty) {
    // Đẩy tín hiệu xuống thanh ghi phần cứng ESP32
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, arm->_ledc_channel, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, arm->_ledc_channel);
}

static void _Robot_Write_PIN(arm_object_t *arm, float angle_rad) {
    float safe_angle = _Robot_Calculate_Servo_Angle(angle_rad);
    uint32_t duty = _Robot_Calculate_Duty(safe_angle);
    _Robot_Set_PWWM_Duty(arm, duty);
}

static void _Robot_Write_All_Pins(robot_object_t *p_robot, theta_t *p_theta) {
    _Robot_Write_PIN(&p_robot->_arm_1, p_theta->arm_1);
    _Robot_Write_PIN(&p_robot->_arm_2, p_theta->arm_2);
    _Robot_Write_PIN(&p_robot->_arm_3, p_theta->arm_3);
}


// ================================ Task Điều Khiển Động Cơ (Motor Control) =================================
void Robot_Motor_Control_Task(void *pvParameters){
    // Khởi tạo Timer cho việc xuất xung PWM
    _Robot_Timer_Init();

    theta_t theta_target = {0, 0, 0};

    // Đưa cánh tay về Home
    _Robot_Write_All_Pins(g_p_robot, &theta_target);

    // ngừng một thời gian để đảm bảo các cánh tay đã về vị trí home trước khi cho phép các task khác hoạt động
    vTaskDelay(pdMS_TO_TICKS(2000)); // Đợi 2 giây

    // Cho phép các task bị dừng trước đó hoạt động
    if (g_handle_planner != NULL)
        vTaskResume(g_handle_planner);
    if (g_handle_kinematics != NULL)
        vTaskResume(g_handle_kinematics);

    // luồng chạy chính của task điều khiển động cơ
    while (1) {
        // Kiểm tra có bộ góc mục tiêu mới nào được gửi từ kinematics đến control hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY)) {
        _Robot_Write_All_Pins(g_p_robot, &theta_target);     
        }
    }
}