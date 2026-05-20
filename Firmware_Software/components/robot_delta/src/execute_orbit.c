#include "execute_orbit.h"

#include "math_until.h"

static float _Apply_Velocity_Profile(float t, velocity_profile_t profile) {
    switch (profile) {
        case PROFILE_ACCEL:  
            return t * t;                               // Chậm -> Nhanh
        case PROFILE_DECEL:  
            return t * (2.0f - t);                      // Nhanh -> Chậm
        case PROFILE_CRUISE: 
            return t;                                   // Tuyến tính (Giữ Vmax)
        case PROFILE_S_CURVE:
        default:             
            return t * t * (3.0f - 2.0f * t);           // Chậm -> Nhanh -> Chậm
    }
}


// Hàm thực thi chạy đường thẳng
void Execute_Linear_Motion(robot_object_t *p_robot, point_t p_start, point_t p_end, float target_speed, velocity_profile_t profile) {
    // Tự động tính thời gian dựa trên tốc độ và quãng đường (Đơn vị: mm và mm/s)
    float distance = Math_Calculate_Distance(&p_start, &p_end);
    uint16_t time_ms = (uint16_t)((distance / target_speed) * 1000.0f);
    
    // Bảo vệ lỗi chia 0 hoặc khoảng cách quá ngắn
    if (time_ms < CYCLE_TIME_MS) time_ms = CYCLE_TIME_MS; 

    uint16_t total_steps = time_ms / CYCLE_TIME_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(CYCLE_TIME_MS);

    for (uint16_t i = 1; i <= total_steps; i++) {
        float t = (float)i / total_steps;
        
        //Ép Profile Vận Tốc
        float t_profiled = _Apply_Velocity_Profile(t, profile);
        
        //Nội suy tọa độ
        point_t p_next = Math_Linear_Interpolation(&p_start, &p_end, t_profiled);

        xSemaphoreTake(p_robot->lock, portMAX_DELAY);
        p_robot->end_effector_target = p_next;
        p_robot->has_end_effector_target_changed = true;
        xSemaphoreGive(p_robot->lock);

        xQueueSend(g_queue_planner_to_kinematics, &p_next, portMAX_DELAY);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Hàm thực thi chạy bo cua
void Execute_Bezier_Motion(robot_object_t *p_robot, point_t p_A, point_t p_C, point_t p_B, float target_speed, velocity_profile_t profile) {
    // Tự động tính thời gian đi qua cua
    float arc_length = Math_Calculate_Bezier_Length(&p_A, &p_C, &p_B);
    uint16_t time_ms = (uint16_t)((arc_length / target_speed) * 1000.0f);
    
    if (time_ms < CYCLE_TIME_MS) time_ms = CYCLE_TIME_MS;

    uint16_t total_steps = time_ms / CYCLE_TIME_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(CYCLE_TIME_MS);

    for (uint16_t i = 1; i <= total_steps; i++) {
        float t = (float)i / total_steps;
        
        //Ép Profile Vận Tốc
        float t_profiled = _Apply_Velocity_Profile(t, profile);
        
        //Nội suy tọa độ đường cong
        point_t p_next = Math_Quadratic_Bezier_Interpolation(&p_A, &p_C, &p_B, t_profiled);

        xSemaphoreTake(p_robot->lock, portMAX_DELAY);
        p_robot->end_effector_target = p_next;
        p_robot->has_end_effector_target_changed = true;
        xSemaphoreGive(p_robot->lock);

        xQueueSend(g_queue_planner_to_kinematics, &p_next, portMAX_DELAY);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}