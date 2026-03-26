#include "math_until.h"

#include <math.h>

point_t Math_Linear_Interpolation(const point_t *p_point_current, const point_t *p_point_end, const float step) {
    point_t point_target;
    point_target.x = p_point_current->x + (p_point_end->x - p_point_current->x) * step;
    point_target.y = p_point_current->y + (p_point_end->y - p_point_current->y) * step;
    point_target.z = p_point_current->z + (p_point_end->z - p_point_current->z) * step;

    return point_target;
}

void Math_Low_Pass_Filter(point_t *p_current, const point_t *p_target) {
    // hệ số làm mượt, để êm ái khi di chuyển chậm (0.0 -> 1.0)
    const float smooth_factor = 0.4f; 
    
    // GIỚI HẠN BƯỚC NHẢY (TỐC ĐỘ MAX): 
    const float max_step = 9.0f; 

    // Tính vector khoảng cách từ vị trí hiện tại đến điểm mục tiêu
    const float dx = p_target->x - p_current->x;
    const float dy = p_target->y - p_current->y;
    const float dz = p_target->z - p_current->z;

    // Tính toán độ dài của bước nhảy nếu chỉ dùng Low-Pass Filter
    float step_x = dx * smooth_factor;
    float step_y = dy * smooth_factor;
    float step_z = dz * smooth_factor;

    // Đo độ dài thực tế của vector bước nhảy này trong không gian 3D
    float step_length = sqrtf(step_x*step_x + step_y*step_y + step_z*step_z);

    // ================== CƠ CHẾ PHANH ABS ==================
    // Nếu bước nhảy tính ra quá lớn, ta bóp nó lại bằng đúng mức max_step!
    if (step_length > max_step) {
        float scale = max_step / step_length;
        step_x *= scale;
        step_y *= scale;
        step_z *= scale;
    }

    // Cập nhật tọa độ an toàn cho Robot (Dùng toán tử -> vì đã đổi sang con trỏ)
    p_current->x += step_x;
    p_current->y += step_y;
    p_current->z += step_z;
}

point_t Math_Get_Parabolic_Arc_Point(point_t *p_point_current, point_t *p_point_end,const float height,const float t) {
    // Nội suy LERP + Parabol
    point_t point_target = Math_Linear_Interpolation(p_point_current, p_point_end, t); // Điểm nội suy tuyến tính ban đầu
    point_target.z += 4.0f * height * t * (1.0f - t);
    
    return point_target;
}