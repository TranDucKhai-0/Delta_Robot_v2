#pragma once

#include "type_data.h"
#include <stdbool.h> // Cần thiết để xài kiểu bool trả về

typedef struct {
    // ==========================================
    int current_step;   // Nhịp đập hiện tại (bắt đầu từ 0)
    int total_steps;    // Tổng số nhịp đập để đi hết 1 quỹ đạo
    int current_type;   // Loại quỹ đạo đang chạy (1: Đường thẳng, 2: Đường tròn)

    // ==========================================
    float z;            // Độ cao Z cố định (VD: -290.0 mm)
    float R;            // Bán kính cấu hình cho vòng tròn Auto (VD: 130.0 mm)
    int auto_state;     // Quản lý kịch bản: 0 (Khởi tạo), 1 (Đang mồi Line), 2 (Lặp Circle)

    // ==========================================
    // Dùng cho hàm Start_Line
    float start_x, start_y, start_z;
    float end_x, end_y, end_z;

    // Dùng cho hàm Start_Circle
    float center_x, center_y, center_z;
    float radius;       // Bán kính thực tế truyền vào hàm vẽ đường tròn
} trajectory_t;

// Truyền cả "não" (traj) để lưu dữ liệu, và "cơ bắp" (robot) để lấy gốc xuất phát
void Start_Line(trajectory_t *p_traj, robot_object_t *p_robot, float end_x, float end_y, float end_z, int total_steps);

// Truyền "não" (traj) và các thông số của đường tròn
void Start_Circle(trajectory_t *p_traj, float center_x, float center_y, float center_z, float radius, int total_steps);

// Hàm xuất tọa độ nội suy (Bắt buộc phải có)
bool Get_Next_Point(trajectory_t *p_traj, point_t *p_point);