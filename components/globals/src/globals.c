#include "globals.h"

// Khởi tạo biến toàn cục chứa trạng thái của robot
robot_object_t *g_p_robot = NULL; // Khởi tạo con trỏ robot thành NULL để đảm bảo an toàn bộ nhớ

// Định nghĩa các Queue (cần thiết để linker tìm thấy vùng nhớ)
QueueHandle_t g_queue_udp_to_planner = NULL;
QueueHandle_t g_queue_planner_to_kinematics = NULL;
QueueHandle_t g_queue_kinematics_to_control = NULL;

TaskHandle_t g_handle_planner = NULL;
TaskHandle_t g_handle_kinematics = NULL;