#include "robot_delta.h"


robot_object_t Robot_Create(const float A, const float RF, const float RE, const float Z_MIN, const float Z_MAX, const float R2){
    robot_object_t robot = {
        .A = A,
        .RF = RF,
        .RE = RE,
        .Z_MIN = Z_MIN,
        .Z_MAX = Z_MAX,
        .R2 = R2,
        ._has_end_effector_current_changed = false, // mặc định chưa có gì thây đổi
        ._has_theta_current_changed = false, // mặc định chưa có gì thây đổi

        ._has_end_effector_target_changed = false, // mặc định chưa có gì thây đổi
        ._has_theta_target_changed = false // mặc định chưa có gì thây đổi

        ._should_break_homing = false // mặc định không cần ngắt homing
    };
    return robot;
}
