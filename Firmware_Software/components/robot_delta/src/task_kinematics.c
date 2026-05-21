#include "task_kinematics.h"

#include "kinematics.h"
#include "math_until.h"

#include "esp_log.h" // Thư viện in Log

// Định nghĩa một cái tên (TAG) để dễ lọc Log trên terminal
static const char *TAG = "Kinematic"; 


// ==============================Task Tính Toán Động Học Ngược===============================
void Robot_Kinematics_Task(void *pvParameters){
    point_t point_target;
    theta_t theta_target; // chứa kết quả tính toán góc theta mục tiêu từ điểm mục tiêu
    while (1) {
        // Kiểm tra có điểm mục tiêu mới nào được gửi từ planner đến kinematics hay không, không thì cho task này ngủ
        if (xQueueReceive(g_queue_planner_to_kinematics, &point_target, portMAX_DELAY)) {

            // kiểm tra điểm mục tiêu có bị trùng điểm hiện tại không
            xSemaphoreTake(g_p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
            if(point_target.x == g_p_robot->end_effector_current.x &&
               point_target.y == g_p_robot->end_effector_current.y &&
               point_target.z == g_p_robot->end_effector_current.z) {
                
                g_p_robot->has_end_effector_target_changed = false;
                xSemaphoreGive(g_p_robot->lock); // Unlock sau khi đã cập nhật cờ
                continue; // bỏ qua vòng lặp này nếu điểm mục tiêu trùng với điểm hiện tại
            }
            xSemaphoreGive(g_p_robot->lock); // Unlock sau khi đã cập nhật cờ
            

            theta_target = Kinematics_Call_Inverse(g_p_robot, &point_target); // Tính toán góc theta mục tiêu từ điểm mục tiêu và cập nhật vào struct robot
            
            xQueueSend(g_queue_kinematics_to_control, &theta_target, portMAX_DELAY); // Gửi góc theta mục tiêu đã tính toán được về planner để planner có thể sử dụng trong quá trình lập kế hoạch di chuyển.

            // ESP_LOGI(TAG, "Đã tín góc (rad): Arm 1: %.2f | Arm 2: %.2f | Arm 3: %.2f", 
            //               theta_target.arm_1, theta_target.arm_2, theta_target.arm_3);

            
            xSemaphoreTake(g_p_robot->lock, portMAX_DELAY); // Lock để đảm bảo an toàn khi truy cập vào robot
            // Chỉ cập nhật tọa độ thực tế nếu tính toán IK thành công (cờ has_theta_target_changed được bật bởi hàm IK)
            if (g_p_robot->has_theta_target_changed) {
                g_p_robot->end_effector_current = point_target; 
                g_p_robot->has_end_effector_current_changed = true; 
                g_p_robot->theta_current = theta_target; 
                g_p_robot->has_theta_current_changed = true; 
                g_p_robot->has_theta_target_changed = false; // Xóa cờ sau khi đã ghi nhận thành công
            }
            xSemaphoreGive(g_p_robot->lock); // Unlock sau khi đã đọc điểm hiện tại
        }
    }
}