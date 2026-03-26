#include "kinematics.h"
#include "math_until.h"

#include <math.h>


static const int8_t _PHI[3] = {0, 120, -120}; // deg

// Xoay hệ tọa độ một góc phi (0, 120, -120) quanh trục Z
// Dùng để đưa bài toán 3 cánh tay về bài toán 1 cánh tay cục bộ
static inline point_t _Passive_Rotation(const point_t *p_point, int8_t phi)
{
    // phi: 0, 120, -120
    if (phi == 0)
    {
        return *p_point;
    }
 
    static const float _c = -0.5f;                       // cos(+-120)
    static const float _s_abs = 0.86602540378f;          // sin(120) = sqrt(3)/2

    point_t p_point_new;

    const float s = (phi > 0) ? _s_abs : -_s_abs;

    p_point_new.x = p_point->x * _c + p_point->y * s;
    p_point_new.y = -p_point->x * s + p_point->y * _c;
    p_point_new.z = p_point->z;

    return p_point_new;
}

// Hàm này sẽ kiểm tra xem điểm mục tiêu có nằm trong vùng hoạt động hiệu quả của robot hay không.
static bool _Robot_Is_In_Workspace(const robot_object_t* p_robot, point_t *p_point){
    // kiểm tra độ cao z
    if (p_point->z > p_robot->Z_MAX || p_point->z < p_robot->Z_MIN)
        return false;

    // kiểm tra bán kính có bé hơn R
    if (sqr(p_point->x) + sqr(p_point->y) > p_robot->R2)
        return false;

    return true;
}

static bool _Calculate_Kinematics_Inverse(const robot_object_t *self, point_t *p_point_target, theta_t *p_theta_target){
    if (!self || !p_point_target || !p_theta_target) return false;
    
    const float RF2 = sqr(self->RF);
    const float RE2 = sqr(self->RE);

    float *theta_future[3] = {&(p_theta_target->arm_1), &(p_theta_target->arm_2), &(p_theta_target->arm_3)};

    for (int8_t i = 0; i < 3; i++)
    {
        point_t E_i_arm_i = _Passive_Rotation(p_point_target, _PHI[i]); // Xoay điểm Eo mục tiêu (End-effector) về hệ tọa độ local của cánh tay thứ i

        E_i_arm_i.x += self->A; // Tọa độ Ei tại từng cánh tay

        const float d = sqrtf(sqr(E_i_arm_i.x - self->A) + sqr(E_i_arm_i.z));
        // Kiểm tra khoảng cách hợp lệ (tránh chia cho 0)
        if (d < 1e-6f)
            return false;

        // Tính toán giao điểm giữa hình cầu (cánh tay dưới re) và vòng tròn (cánh tay trên rf)
        const float p = (RF2 - RE2 + sqr(E_i_arm_i.y) + sqr(d)) / (2 * d);
        
        // check
        if ((p > self->RF) || (p < 0))
            return false;

        const float H = sqrtf(RF2 - sqr(p));

        const point_t P_i = {self->A + p * (E_i_arm_i.x - self->A) / d,
                                0.0f,
                                p * (E_i_arm_i.z) / d};

        // Vector vuông góc để tìm 2 nghiệm giao điểm
        const float vector[2] = {-E_i_arm_i.z, E_i_arm_i.x - self->A};

        point_t j[2];

        j[0].x = P_i.x + (H / d) * vector[0]; // x1
        j[0].z = P_i.z + (H / d) * vector[1]; // z1

        j[1].x = P_i.x - (H / d) * vector[0]; // x2
        j[1].z = P_i.z - (H / d) * vector[1]; // z2

        // chọn nghiệm thỏa điều kiện
        if ((j[0].x >= self->A) && (j[0].z <= 0))
            *theta_future[i] = atan2f(j[0].z, j[0].x - self->A);

        else if ((j[1].x >= self->A) && (j[1].z <= 0))
            *theta_future[i] = atan2f(j[1].z, j[1].x - self->A);

        else
            return false;
    }
    return true;
}


static bool _Calculate_Kinematics_Forward(const robot_object_t *self, theta_t *p_theta_target, point_t *p_point_target){
    // Kểm tra con trỏ null để bảo vệ bộ nhớ
    if (!self || !p_theta_target || !p_point_target) return false;
    
    // Trích xuất các biến từ struct vào mảng để tận dụng vòng lặp
    const float theta_array[3] = {
        p_theta_target->arm_1,
        p_theta_target->arm_2,
        p_theta_target->arm_3
    };
    
    // biến chứa tọa độ của các khớp nối trong hệ trục cục bộ và global
    point_t j_local[3];
    point_t j_global[3];

    // Tính vị trí của các khớp nối trong hệ trục cục bộ của từng arm
    for (int8_t i = 0; i < 3; i++) {
        float sin_val, cos_val; 
        
        // Dùng sincosf để tối ưu hiệu suất, hoặc thay bằng sinf()/cosf() nếu compiler không hỗ trợ
        sincosf(theta_array[i], &sin_val, &cos_val);

        j_local[i].x = self->A + self->RF * cos_val;
        j_local[i].y = 0.0f;
        j_local[i].z = self->RF * sin_val;

        // Xoay hệ trục từng arm ngược về hệ global
        // Truyền _PHI[i] (giả sử _PHI là mảng toàn cục chứa 3 góc của trục)
        j_global[i] = _Passive_Rotation(&j_local[i], -_PHI[i]); 
    }

    const float A1 = 2 * (j_global[0].x - j_global[1].x); 
    const float B1 = 2 * (j_global[1].y);                  
    const float C1 = 2 * (j_global[0].z - j_global[1].z); 
    const float D1 = sqr(j_global[0].x) - sqr(j_global[1].x) - sqr(j_global[1].y) + sqr(j_global[0].z) - sqr(j_global[1].z);

    const float A2 = 2 * (j_global[0].x - j_global[2].x); 
    const float B2 = 2 * (j_global[2].y);                  
    const float C2 = 2 * (j_global[0].z - j_global[2].z); 
    const float D2 = sqr(j_global[0].x) - sqr(j_global[2].x) - sqr(j_global[2].y) + sqr(j_global[0].z) - sqr(j_global[2].z);

    const float denom = A1 * B2 - A2 * B1;
    // Kiểm tra mẫu số (tránh chia cho 0)
    if (fabsf(denom) < 1e-6f)
        return false;

    const float A = (-D1 * B2 + D2 * B1) / denom;
    const float C = (A1 * D2 - A2 * D1) / denom;
    const float B = (-C1 * B2 + C2 * B1) / denom;
    const float D = (A1 * C2 - A2 * C1) / denom;

    const float A_ = A - j_global[0].x;

    const float b = sqr(B) + sqr(D) + 1;
    const float c = 2 * B * A_ + 2 * C * D - 2 * j_global[0].z;
    const float d = sqr(A_) + sqr(C) + sqr(j_global[0].z) - sqr(self->RE);

    const float delta = sqr(c) - 4 * b * d;
    if (delta < 0)
        return false;

    // Chọn nghiệm z thấp hơn (Robot Delta hướng xuống)
    const float z0 = fminf((-c + sqrtf(delta)) / (2 * b), (-c - sqrtf(delta)) / (2 * b));

    p_point_target->x = A + B * z0;
    p_point_target->y = C + D * z0;
    p_point_target->z = z0;

    return true;
}



// Hàm này sẽ gọi hàm kinematics_inverse để tính toán góc theta từ điểm mục tiêu, và cập nhật trạng thái của robot.
theta_t Kinematics_Call_Inverse(robot_object_t* p_robot, point_t *p_point_target){
    if(p_robot->_has_end_effector_target_changed) { 
        theta_t theta_target; // chứa kết quả tính toán góc theta mục tiêu từ điểm mục tiêu
        // Nếu điểm mục tiêu không nằm trong vùng hoạt động hiệu quả, nó sẽ đặt cờ _has_end_effector_target_changed thành false để báo hiệu rằng điểm đó không hợp lệ.
        if (!_Robot_Is_In_Workspace(p_robot, p_point_target)){}

        else if(_Calculate_Kinematics_Inverse(p_robot, p_point_target, &theta_target)){
            p_robot->_has_theta_current_changed = true; // Đặt cờ này thành true để báo hiệu rằng góc theta đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }

        p_robot->_has_end_effector_target_changed = false; // Đặt cờ này thành false sau khi đã dùng tọa độ mục tiêu, để tránh tính toán lại nếu điểm mục tiêu không thay đổi.
        p_robot->theta_target = theta_target; // Cập nhật góc theta mục tiêu vào struct robot để các phần khác của hệ thống có thể truy cập và sử dụng.
    }
    // Trả về góc mục tiêu trong robot để nếu ko tính FK dc thì lấy góc cũ
    return p_robot->theta_target;
}



// Hàm này sẽ gọi hàm kinematics_forward để tính toán vị trí của end-effector từ góc theta mục tiêu
point_t Kinematics_Call_Forward(robot_object_t* p_robot, theta_t *p_theta_target){
    if(p_robot->_has_theta_target_changed) {
        point_t point_target; // chứa kết quả tính toán vị trí end-effector từ góc theta mục tiêu
        
        if(_Calculate_Kinematics_Forward(p_robot, p_theta_target, &point_target)){
            p_robot->_has_end_effector_current_changed = true; // Đặt cờ này thành true để báo hiệu rằng vị trí end-effector đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        }

        p_robot->_has_theta_target_changed = false; // Đặt cờ này thành false sau khi đã dùng bộ gốc mục tiêu, để tránh tính toán lại nếu góc theta không thay đổi.
        p_robot->end_effector_target = point_target; // Cập nhật vị trí end-effector vào struct robot để các phần khác của hệ thống có thể truy cập và sử dụng.
    }
    // Trả về vị trí end-effector mục tiêu trong robot để nếu ko tính FK dc thì lấy vị trí cũ
    return p_robot->end_effector_target;
}