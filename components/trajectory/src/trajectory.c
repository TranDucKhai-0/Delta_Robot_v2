#include "trajectory.h"
#include <math.h>

void Start_Line(trajectory_t *p_traj, robot_object_t *p_robot, float end_x, float end_y, float end_z, int total_steps) {
    // Tự động lấy vị trí ĐANG ĐỨNG THỰC TẾ của robot làm điểm xuất phát
    p_traj->start_x = p_robot->end_effector_current.x;
    p_traj->start_y = p_robot->end_effector_current.y;
    p_traj->start_z = p_robot->end_effector_current.z;

    // Nạp điểm đến
    p_traj->end_x = end_x;
    p_traj->end_y = end_y;
    p_traj->end_z = end_z;

    // Setup thông số Engine
    p_traj->total_steps = total_steps;
    p_traj->current_step = 0;
    p_traj->current_type = 1; // Bật cờ cắm mốc là đang chạy Line
}

void Start_Circle(trajectory_t *p_traj, float center_x, float center_y, float center_z, float radius, int total_steps) {
    // Nạp thông số hình học
    p_traj->center_x = center_x;
    p_traj->center_y = center_y;
    p_traj->center_z = center_z;
    p_traj->radius = radius;

    // Setup thông số Engine
    p_traj->total_steps = total_steps;
    p_traj->current_step = 0;
    p_traj->current_type = 2; // Bật cờ cắm mốc là đang chạy Circle
}

// BỔ SUNG: Thuật toán nội suy nhả tọa độ theo từng nhịp
bool Get_Next_Point(trajectory_t *p_traj, point_t *p_point) {
    if (p_traj->current_step > p_traj->total_steps || p_traj->current_type == 0) {
        return false; 
    }

    float t = (float)p_traj->current_step / (float)p_traj->total_steps;

    if (p_traj->current_type == 1) { 
        p_point->x = p_traj->start_x + (p_traj->end_x - p_traj->start_x) * t;
        p_point->y = p_traj->start_y + (p_traj->end_y - p_traj->start_y) * t;
        p_point->z = p_traj->start_z + (p_traj->end_z - p_traj->start_z) * t;
    } 
    else if (p_traj->current_type == 2) { 
        float angle = t * 2.0f * (float)M_PI; 
        float sin_val, cos_val; 
        sincosf(angle, &sin_val, &cos_val); 
        
        p_point->x = p_traj->center_x + p_traj->radius * cos_val;
        p_point->y = p_traj->center_y + p_traj->radius * sin_val;
        p_point->z = p_traj->center_z;
    }
    
    p_traj->current_step++;
    return true; 
}