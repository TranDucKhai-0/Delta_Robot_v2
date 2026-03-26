#include "task_planner.h"

#include "type_data.h"
#include "globals.h"
#include "robot_delta.h"
#include "kinematics.h"
#include "math_until.h"
#include "trajectory.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h" // Thư viện in Log

// Định nghĩa một cái tên (TAG) để dễ lọc Log trên terminal
static const char *TAG = "PLANNER"; 

static trajectory_t _auto_traj = {0}; 
static bool _is_traj_initialized = false;


static void _Robot_Homing(robot_object_t *p_robot) {
    uint8_t homing_step = 50;
    theta_t theta_home = {0.0f, 0.0f, 0.0f}; // Góc theta home (có thể điều chỉnh tùy theo cấu hình robot)

    p_robot->has_theta_target_changed = true; // Đặt cờ này thành true để báo hiệu rằng góc theta mục tiêu đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
    point_t point_home = Kinematics_Call_Forward(p_robot, &theta_home); // Tính toán điểm home từ góc theta home
    point_t point_target; // chứa kết quả nội suy

    for (uint8_t i = 0; i < homing_step; i++) {
        xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
        bool should_break_homing = p_robot->should_break_homing; // Đọc cờ break homing vào biến cục bộ
        xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ
        
        // Kiểm tra cờ break homing trước khi tiếp tục nội suy
        if(should_break_homing) {
            xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
            p_robot->should_break_homing = false; // Reset cờ sau khi đã dùng
            xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ
            return; // Thoát khỏi quá trình homing nếu cờ break được đặt
        }

        // Nội suy tuyến tính từ theta_current về theta_home
        point_target = Math_Linear_Interpolation(&p_robot->end_effector_current, &point_home, (float)i / homing_step);
        point_target.mode = 0;

        // === IN LOG TỌA ĐỘ VỀ HOME ===
        // ESP_LOGI(TAG, "Đã tín Tọa độ về HOME: MODE: %d | X: %.2f | Y: %.2f | Z: %.2f", 
        //                       point_target.mode, point_target.x, point_target.y, point_target.z);

        // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
        xQueueSend(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY);

        // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void _Robot_Automatic(robot_object_t *p_robot) {
    if (!_is_traj_initialized) {
        _auto_traj.R = 130.0f;
        _auto_traj.z = -290.0f;
        _auto_traj.auto_state = 0;
        
        Start_Line(&_auto_traj, p_robot, _auto_traj.R, 0.0f, _auto_traj.z, 50);
                    
        _auto_traj.auto_state = 1;
        _is_traj_initialized = true;
    }

    point_t auto_point;
    bool has_next = Get_Next_Point(&_auto_traj, &auto_point);

    if (!has_next) {
        if (_auto_traj.auto_state == 1 || _auto_traj.auto_state == 2) {
            Start_Circle(&_auto_traj, 0.0f, 0.0f, _auto_traj.z, _auto_traj.R, 100);
            _auto_traj.auto_state = 2;
        }
        Get_Next_Point(&_auto_traj, &auto_point);
    }

    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->end_effector_target = auto_point;
    p_robot->has_end_effector_target_changed = true; 
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ
    
    // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
    xQueueSend(g_queue_planner_to_kinematics, &auto_point, portMAX_DELAY);

    // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
    vTaskDelay(pdMS_TO_TICKS(20));
}

static void _Robot_Manual(robot_object_t *p_robot, point_t *p_point_target) {
    point_t point_current;
    Math_Low_Pass_Filter(&point_current, p_point_target); // Áp dụng bộ lọc thông thấp để làm mượt chuyển động
    
    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    p_robot->end_effector_target = point_current;
    p_robot->has_end_effector_target_changed = true; 
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

    // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
    xQueueSend(g_queue_planner_to_kinematics, &point_current, portMAX_DELAY);
}

static void _Robot_Pick_And_Place(robot_object_t *p_robot, point_t *p_point_target) {
    if (p_robot == NULL || p_point_target == NULL) {
        return; 
    }

    // Thông số thời gian & quỹ đạo (Đại Ca có thể đưa vào tham số hàm nếu cần)
    const uint16_t TOTAL_TIME_MS = 1500;    // Tổng thời gian di chuyển
    const uint16_t CYCLE_TIME_MS = 20;      // Chu kỳ nội suy: 20ms (tương đương 50Hz)
    const float CLEARANCE_HEIGHT = p_robot->Z_MAX - p_robot->Z_MIN - 1.0f;   // Độ cao nhấc vật (parabol)

    const uint16_t TOTAL_STEPS = TOTAL_TIME_MS / CYCLE_TIME_MS; // Tính tổng số bước nội suy dựa trên tổng thời gian và chu kỳ nội suy

    // Khởi tạo biến thời gian cho vTaskDelayUntil
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(CYCLE_TIME_MS);

    xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
    point_t point_current = p_robot->end_effector_current; // Lấy điểm hiện tại của end-effector
    xSemaphoreGive(p_robot->lock); // Unlock sau khi đã đọc điểm hiện tại

    for (uint16_t i = 0; i <= TOTAL_STEPS; i++) {
        float t = (float)i / TOTAL_STEPS; 

        // Tính toán và trả về tọa độ tại bước t
        point_t point_next = Math_Get_Parabolic_Arc_Point(&point_current, p_point_target, CLEARANCE_HEIGHT, t);

        xSemaphoreTake(p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
        p_robot->end_effector_target = point_next; // Cập nhật điểm mục tiêu    
        p_robot->has_end_effector_target_changed = true; // Đặt cờ này thành true để báo hiệu rằng điểm mục tiêu đã thay đổi và cần được cập nhật trong hệ thống điều khiển.
        xSemaphoreGive(p_robot->lock); // Unlock sau khi đã cập nhật cờ

        xQueueSend(g_queue_planner_to_kinematics, &point_next, portMAX_DELAY); // Gửi điểm nội suy đến Task Kinematics để điều khiển động cơ

        // Quản lý thời gian thực
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


// =============================Task Planner =============================
void Robot_Planner_Task(void *pvParameters){
    point_t point_target = {0.0f, 0.0f, 0.0f, 0}; // chứa điểm mới từ PC
    point_t point_current; // chứa điểm trước đó

    while (1) {
        //Kiểm tra xem có tọa độ mới từ máy tính không (Không có thì lấy điểm cũ)
        xQueueReceive(g_queue_udp_to_planner, &point_target, portMAX_DELAY);
        
        point_current.mode = point_target.mode; // Cập nhật mode mới

        if(point_current.mode == MODE_HOMING) {
            _Robot_Homing(g_p_robot); // Thực hiện quá trình homing
        } 
        else if(point_current.mode == MODE_AUTOMATIC){
            _Robot_Automatic(g_p_robot); // thực hiện quá trình tự sinh quỹ đạo theo kịch bản
        } 
        else if(point_current.mode == MODE_MANUAL){
            _Robot_Manual(g_p_robot, &point_target); // thực hiện quá trình điều khiển thủ công theo tọa độ từ PC
        } 
        else if(point_current.mode == MODE_PICK_AND_PLACE){
            _Robot_Pick_And_Place(g_p_robot, &point_target); // thực hiện quá trình pick and place theo tọa độ từ PC
        }
        //Ngừng 20ms để cho task khác trong Core0 hoạt động
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}