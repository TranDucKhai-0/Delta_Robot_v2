#include "globals.h"

// Định nghĩa các Queue (cần thiết để linker tìm thấy vùng nhớ)
QueueHandle_t g_queue_udp_to_planner = NULL;
QueueHandle_t g_queue_planner_to_core1 = NULL;