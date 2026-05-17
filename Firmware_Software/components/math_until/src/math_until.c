#include "math_until.h"

#include <math.h>

float Math_Calculate_Distance(const point_t *p1, const point_t *p2) {
    float dx = p2->x - p1->x;
    float dy = p2->y - p1->y;
    float dz = p2->z - p1->z;
    return sqrtf(sqr(dx) + sqr(dy) + sqr(dz));
}

// Hàm nội tuyến tính (Linear Interpolation)
point_t Math_Linear_Interpolation(const point_t *p_start, const point_t *p_end, const float t) {
    point_t p_out;
    p_out.x = p_start->x + (p_end->x - p_start->x) * t;
    p_out.y = p_start->y + (p_end->y - p_start->y) * t;
    p_out.z = p_start->z + (p_end->z - p_start->z) * t;
    return p_out;
}

void Math_Low_Pass_Filter(point_t *p_current, const point_t *p_target) {
    // hệ số làm mượt, để êm ái khi di chuyển chậm (0.0 -> 1.0)
    const float smooth_factor = 1.0f;
     

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
    float step_length = sqrtf(sqr(step_x) + sqr(step_y) + sqr(step_z));

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

// Hàm nội suy đường cong bo góc (Quadratic Bezier Curve)
point_t Math_Quadratic_Bezier_Interpolation(const point_t *p_A, const point_t *p_C, const point_t *p_B, const float t) {
    float u = 1.0f - t;
    float tt = sqr(t);
    float uu = sqr(u);
    float u2t = 2.0f * u * t;

    point_t p_out;
    p_out.x = uu * p_A->x + u2t * p_C->x + tt * p_B->x;
    p_out.y = uu * p_A->y + u2t * p_C->y + tt * p_B->y;
    p_out.z = uu * p_A->z + u2t * p_C->z + tt * p_B->z;
    return p_out;
}

// Hàm tính toán tự động: Tìm điểm cắt góc để bắt đầu/kết thúc đường cong Bézier
point_t Math_Move_Towards(const point_t *p_corner, const point_t *p_target, float radius) {
    float len = Math_Calculate_Distance(p_corner, p_target);
    point_t p_out = *p_corner;
    if (len > 0.001f) {
        p_out.x += ((p_target->x - p_corner->x) / len) * radius;
        p_out.y += ((p_target->y - p_corner->y) / len) * radius;
        p_out.z += ((p_target->z - p_corner->z) / len) * radius;
    }
    return p_out;
}

// Tính xấp xỉ chiều dài đường cong Bezier (Bằng cách chia làm 4 đoạn thẳng nhỏ)
float Math_Calculate_Bezier_Length(const point_t *p_A, const point_t *p_C, const point_t *p_B) {
    float length = 0.0f;    
    point_t prev_p = *p_A;
    for(int i = 1; i <= 4; i++) {
        float t = (float)i / 4.0f;
        point_t curr_p = Math_Quadratic_Bezier_Interpolation(p_A, p_C, p_B, t);
        length += Math_Calculate_Distance(&prev_p, &curr_p);
        prev_p = curr_p;
    }
    return length;
}

// point_t Math_Get_Parabolic_Arc_Point(point_t *p_point_current, point_t *p_point_end,const float height,const float t) {
//     point_t point_target;
    
//     // Nội suy LERP + Parabol
//     // point_target = Math_Linear_Interpolation(p_point_current, p_point_end, t); // Điểm nội suy tuyến tính ban đầu
//     // point_target.z += 4.0f * height * t * (1.0f - t);

    


//     return point_target;
// }