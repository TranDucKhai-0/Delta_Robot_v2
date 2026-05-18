#include "task_planner.h"

#include "type_data.h"
#include "globals.h"
#include "robot_delta.h"
#include "kinematics.h"
#include "math_until.h"
#include "trajectory.h"
#include "execute_orbit.h"
#include "gripper.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h" // Thư viện in Log

// Định nghĩa một cái tên (TAG) để dễ lọc Log trên terminal
static const char *TAG = "PLANNER";

static trajectory_t _auto_traj = {0};
static bool _is_traj_initialized = false;

// Tạo biến quản lý gripper
gripper_object_t g_gripper = { .GPIO_INDX = GRIPPER, .TIME_DELAY_MS = GRIPPER_TIME_DELAY_MS}; // Cấu hình GPIO và thời gian
Gripper_Init(&g_gripper, GRIPPER_DEFAULT_STATE); // Khởi tạo gripper với trạng thái nhả

static void _Robot_Homing(robot_object_t *p_robot)
{
    uint8_t homing_step = 50;
    theta_t theta_home = {0.0f, 0.0f, 0.0f}; // Góc theta home (có thể điều chỉnh tùy theo cấu hình robot)

    point_t point_home = {.x = theta_home.arm_1, .y = theta_home.arm_1, .z = theta_home.arm_1}; // theta home
    point_t point_target;                                                                       // chứa kết quả nội suy

    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    theta_t theta_current = p_robot->theta_current;
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

    point_t point_current = {.x = theta_current.arm_1, .y = theta_current.arm_1, .z = theta_current.arm_1}; // chứa góc hiện tại

    for (uint8_t i = 0; i <= homing_step; i++)
    {
        xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
        // Kiểm tra cờ break homing trước khi tiếp tục nội suy
        if (p_robot->should_break_homing)
        {
            p_robot->should_break_homing = false; // Reset cờ sau khi đã dùng
            xSemaphoreGive(p_robot->lock);        // Unlock sau khi đã cập nhật cờ
            return;                               // Thoát khỏi quá trình homing nếu cờ break được đặt
        }
        xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

        // Nội suy tuyến tính từ theta_current về theta_home
        point_target = Math_Linear_Interpolation(&point_current, &point_home, (float)i / (float)homing_step);

        theta_t theta_target = {.arm_1 = point_target.x, .arm_2 = point_target.y, .arm_3 = point_target.z};

        // ESP_LOGI(TAG, "Đã tín Tọa độ về HOME: X: %.2f | Y: %.2f | Z: %.2f",
        //                       point_target.x, point_target.y, point_target.z);

        xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
        p_robot->theta_current = theta_target;
        p_robot->has_theta_current_changed = true; // Đặt cờ này thành true để báo hiệu rằng góc theta mục tiêu đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        xSemaphoreGive(p_robot->lock);             // Unlock sau khi đã cập nhật cờ

        // === IN LOG TỌA ĐỘ VỀ HOME ===
        // ESP_LOGI(TAG, "Đã tín Tọa độ về HOME: MODE: %d | X: %.2f | Y: %.2f | Z: %.2f",
        //                       point_target.mode, point_target.x, point_target.y, point_target.z);

        // Gửi góc theta nội suy đến Task Control để điều khiển động cơ
        xQueueSend(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY);

        // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
        vTaskDelay(pdMS_TO_TICKS(20));

    // Cập nhật lại tọa độ hệ Đề Cát (end_effector_current) đồng bộ với góc theta vừa đưa về Home
    Robot_Setup_Home_Point(p_robot, &theta_home);
    }
}

static void _Robot_Automatic(robot_object_t *p_robot)
{
    if (!_is_traj_initialized)
    {
        _auto_traj.R = 130.0f;
        _auto_traj.z = -310.0f;
        _auto_traj.auto_state = 0;

        Start_Line(&_auto_traj, p_robot, _auto_traj.R, 0.0f, _auto_traj.z, 50);

        _auto_traj.auto_state = 1;
        _is_traj_initialized = true;
    }

    point_t auto_point;
    bool has_next = Get_Next_Point(&_auto_traj, &auto_point);

    if (!has_next)
    {
        if (_auto_traj.auto_state == 1 || _auto_traj.auto_state == 2)
        {
            Start_Circle(&_auto_traj, 0.0f, 0.0f, _auto_traj.z, _auto_traj.R, 100);
            _auto_traj.auto_state = 2;
        }
        Get_Next_Point(&_auto_traj, &auto_point);
    }

    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->end_effector_target = auto_point;
    p_robot->has_end_effector_target_changed = true;
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

    // === IN LOG TỌA ĐỘ VỀ HOME ===
    // ESP_LOGI(TAG, "Đã tín Tọa độ Auto: MODE: %d | X: %.2f | Y: %.2f | Z: %.2f",
    //                       auto_point.mode, auto_point.x, auto_point.y, auto_point.z);

    // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
    xQueueSend(g_queue_planner_to_kinematics, &auto_point, portMAX_DELAY);

    // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
    vTaskDelay(pdMS_TO_TICKS(20));
}

static void _Robot_Manual(robot_object_t *p_robot, point_t *p_point_target)
{
    xSemaphoreTake(p_robot->lock, portMAX_DELAY);          // Lock để đảm bảo an toàn khi truy cập vào robot
    point_t point_current = p_robot->end_effector_current; // Lấy điểm hiện tại của end-effector
    xSemaphoreGive(p_robot->lock);                         // Unlock sau khi đã đọc điểm hiện tại

    Math_Low_Pass_Filter(&point_current, p_point_target); // Áp dụng bộ lọc thông thấp để làm mượt chuyển động

    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->end_effector_target = point_current;
    p_robot->has_end_effector_target_changed = true;
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

    // ESP_LOGI(TAG, "Đã tín Tọa độ Manual: MODE: %d | X: %.2f | Y: %.2f | Z: %.2f",
    //                       point_current.mode, point_current.x, point_current.y, point_current.z);

    // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
    xQueueSend(g_queue_planner_to_kinematics, &point_current, portMAX_DELAY);
}

static void _Robot_Pick_And_Place(robot_object_t *p_robot, point_t *p_point_pick, point_t *p_point_place) {
    if (p_robot == NULL || p_point_pick == NULL || p_point_place == NULL) return;

    const float Z_SAFE = p_robot->Z_MAX - 1.0f;
    const float R = 3.0f; // Bán kính bo góc (mm)
    
    // TỐC ĐỘ DI CHUYỂN CHUNG (mm/giây)
    //const float SPEED_MM_PER_SEC = 150.0f; 

    // Các điểm neo chính
    point_t p_home = {.x = 0.0f, .y = 0.0f, .z = Z_SAFE, .mode = MODE_PICK_AND_PLACE};
    point_t p_above_pick = {.x = p_point_pick->x, .y = p_point_pick->y, .z = Z_SAFE, .mode = MODE_PICK_AND_PLACE};
    point_t p_above_place = {.x = p_point_place->x, .y = p_point_place->y, .z = Z_SAFE, .mode = MODE_PICK_AND_PLACE};
    point_t p_A, p_B;

    // ------------------------------------------------------------------------
    // CHU TRÌNH 1: TỪ HOME TỚI GẮP (Tăng tốc -> Bay -> Giảm tốc cắm xuống)
    // ------------------------------------------------------------------------
    p_A = Math_Move_Towards(&p_above_pick, &p_home, R);
    p_B = Math_Move_Towards(&p_above_pick, p_point_pick, R);

    Execute_Linear_Motion(p_robot, p_home, p_A, SPEED_MM_PER_SEC, PROFILE_ACCEL);
    
    Execute_Bezier_Motion(p_robot, p_A, p_above_pick, p_B, SPEED_MM_PER_SEC, PROFILE_CRUISE);

    Execute_Linear_Motion(p_robot, p_B, *p_point_pick, SPEED_MM_PER_SEC, PROFILE_DECEL);

    // TODO: BẬT GRIPPER TẠI ĐÂY
    Gripper_Change(&g_gripper); // thây đổi trạng thái gripper

    // ------------------------------------------------------------------------
    // CHU TRÌNH 2: TỪ GẮP SANG THẢ (Tăng tốc -> Bay -> Giảm tốc cắm xuống)
    // ------------------------------------------------------------------------
    // 1. Rút lên từ điểm gắp
    p_A = Math_Move_Towards(&p_above_pick, p_point_pick, R);
    Execute_Linear_Motion(p_robot, *p_point_pick, p_A, SPEED_MM_PER_SEC, PROFILE_ACCEL);

    // 2. Ôm cua tại đỉnh Gắp hướng về đỉnh Thả
    p_B = Math_Move_Towards(&p_above_pick, &p_above_place, R);
    Execute_Bezier_Motion(p_robot, p_A, p_above_pick, p_B, SPEED_MM_PER_SEC, PROFILE_CRUISE);

    // 3. Lướt ngang trên không (Chạy Vmax không đổi)
    p_A = Math_Move_Towards(&p_above_place, &p_above_pick, R);
    Execute_Linear_Motion(p_robot, p_B, p_A, SPEED_MM_PER_SEC, PROFILE_CRUISE);

    // 4. Ôm cua tại đỉnh Thả cắm thẳng xuống
    p_B = Math_Move_Towards(&p_above_place, p_point_place, R);
    Execute_Bezier_Motion(p_robot, p_A, p_above_place, p_B, SPEED_MM_PER_SEC, PROFILE_CRUISE);

    // 5. Cắm xuống tới đích và từ từ dừng lại
    Execute_Linear_Motion(p_robot, p_B, *p_point_place, SPEED_MM_PER_SEC, PROFILE_DECEL);

    // TODO: TẮT GRIPPER TẠI ĐÂY
    Gripper_Change(&g_gripper); // thây đổi trạng thái gripper

    // ------------------------------------------------------------------------
    // CHU TRÌNH 3: TỪ THẢ VỀ LẠI HOME
    // ------------------------------------------------------------------------
    p_A = Math_Move_Towards(&p_above_place, p_point_place, R);
    Execute_Linear_Motion(p_robot, *p_point_place, p_A, SPEED_MM_PER_SEC, PROFILE_ACCEL);

    p_B = Math_Move_Towards(&p_above_place, &p_home, R);
    Execute_Bezier_Motion(p_robot, p_A, p_above_place, p_B, SPEED_MM_PER_SEC, PROFILE_CRUISE);

    p_A = Math_Move_Towards(&p_home, &p_above_place, R);
    Execute_Linear_Motion(p_robot, p_B, p_A, SPEED_MM_PER_SEC, PROFILE_DECEL);
}

// =============================Task Planner =============================
void Robot_Planner_Task(void *pvParameters)
{
    // Sử dụng cấu trúc "Hộp" chứa cả 2 dạng dữ liệu
    udp_payload_t payload = {0}; 

    // Khởi tạo gripper với trạng thái nhả trước khi vào vòng lặp vô tận
    Gripper_Init(&g_gripper, GRIPPER_DEFAULT_STATE);

    while (1)
    {
        // Nhận gói tin tổng hợp từ máy tính
        xQueueReceive(g_queue_udp_to_planner, &payload, portMAX_DELAY);

        // Bóc tách CẤU TRÚC 1 ra để phục vụ Mode 0, 1, 2
        point_t point_current = payload.target; 

        if (point_current.mode == MODE_HOMING)
        {
            _Robot_Homing(g_p_robot); // Thực hiện quá trình homing
        }
        else if (point_current.mode == MODE_AUTOMATIC)
        {
            _Robot_Automatic(g_p_robot); // thực hiện quá trình tự sinh quỹ đạo
        }
        else if (point_current.mode == MODE_MANUAL)
        {
            _Robot_Manual(g_p_robot, &point_current); // điều khiển thủ công theo CẤU TRÚC 1
        }
        else if (point_current.mode == MODE_PICK_AND_PLACE)
        {
            // Bóc tách CẤU TRÚC 2 ra để phục vụ riêng Mode 3
            _Robot_Pick_And_Place(g_p_robot, &payload.pick, &payload.place); 
        }
        
        // Ngừng 20ms để cho task khác trong Core0 hoạt động
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}