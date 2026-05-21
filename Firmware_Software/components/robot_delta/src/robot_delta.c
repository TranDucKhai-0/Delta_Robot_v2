#include "robot_delta.h"

#include "kinematics.h"

#include "esp_log.h" // Thư viện in Log


robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2){
    robot_object_t robot = {
        .A = A,
        .RF = RF,
        .RE = RE,
        .Z_MIN = Z_MIN,
        .Z_MAX = Z_MAX,
        .R2 = R2,

        .is_automatic_mode = false, // mặc định khởi động ở chế độ homing

        .has_end_effector_current_changed = false, // mặc định chưa có gì thây đổi
        .has_theta_current_changed = false, // mặc định chưa có gì thây đổi

        .has_end_effector_target_changed = false, // mặc định chưa có gì thây đổi
        .has_theta_target_changed = false, // mặc định chưa có gì thây đổi

        .should_break_homing = false // mặc định không cần ngắt homing
    };
    return robot;
}

static const char *TAG = "Control"; 

// Hàm này chạy khi esp32 vừa khởi động để đặt điểm/góc ban đầu
void Robot_Setup_Home_Point(robot_object_t *p_robot, theta_t *p_theta_home) {
    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->has_theta_target_changed = true; // bật cờ để tính toán lần đầu khi khởi động
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ


    point_t point_home = Kinematics_Call_Forward(p_robot, p_theta_home); // Tính toán điểm home từ góc theta home
    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->end_effector_current = point_home;
    p_robot->theta_current = *p_theta_home;
    p_robot->has_end_effector_current_changed = false; 
    p_robot->has_theta_current_changed = false;
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

    ESP_LOGI(TAG, "Đã tín Tọa độ điểm HOME: X: %.2f | Y: %.2f | Z: %.2f", 
                            point_home.x, point_home.y, point_home.z);
}