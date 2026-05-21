#include "task_automatic.h"

#include "trajectory.h"
#include "globals.h"

static trajectory_t _auto_traj = {0};
static bool _is_traj_initialized = false;

void Robot_Automatic_Task(void *pvParameters) {
    
    while (1)
    {

        xSemaphoreTake(g_p_robot->lock, portMAX_DELAY);          // Lock để đảm bảo an toàn khi truy cập vào robot
        bool is_automatic_mode = g_p_robot->is_automatic_mode;  // Kiểm tra cờ
        xSemaphoreGive(g_p_robot->lock);                         // Unlock sau khi đã đọc điểm hiện tại


        if (is_automatic_mode) {
            if (!_is_traj_initialized)
            {
                _auto_traj.R = 130.0f;
                _auto_traj.z = -310.0f;
                _auto_traj.auto_state = 0;

                Start_Line(&_auto_traj, g_p_robot, _auto_traj.R, 0.0f, _auto_traj.z, 50);

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

            xSemaphoreTake(g_p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
            g_p_robot->end_effector_target = auto_point;
            g_p_robot->has_end_effector_target_changed = true;
            xSemaphoreGive(g_p_robot->lock); // Unlock sau khi đã cập nhật cờ

            // === IN LOG TỌA ĐỘ  ===
            // ESP_LOGI(TAG, "Đã tín Tọa độ Auto: MODE: %d | X: %.2f | Y: %.2f | Z: %.2f",
            //                       auto_point.mode, auto_point.x, auto_point.y, auto_point.z);

            // Gửi góc theta nội suy đến Task Kinematics để điều khiển động cơ
            xQueueSend(g_queue_planner_to_kinematics, &auto_point, portMAX_DELAY);

        }
        else {
            _is_traj_initialized = false; // Reset lại cấu hình nếu thoát khỏi chế độ Auto
        }
        // Ngừng 20ms để cho Task Kinematics thực hiện lệnh và tránh gửi quá nhanh
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
