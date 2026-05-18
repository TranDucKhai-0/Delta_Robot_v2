#include "app.h"
#include "wifi_app.h"
#include "over_the_air.h"
#include "globals.h"
#include "robot_delta.h"
#include "udp_receive.h"
#include "type_data.h"
#include "arm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Khởi tạo biến toàn cục chứa trạng thái của robot
robot_object_t *g_p_robot = NULL; // Khởi tạo con trỏ robot thành NULL để đảm bảo an toàn bộ nhớ

// Định nghĩa các Queue (cần thiết để linker tìm thấy vùng nhớ)
QueueHandle_t g_queue_udp_to_planner = NULL;
QueueHandle_t g_queue_planner_to_kinematics = NULL;
QueueHandle_t g_queue_kinematics_to_control = NULL;

TaskHandle_t g_handle_planner = NULL;
TaskHandle_t g_handle_kinematics = NULL;


// Hàm khởi tạo các biến toàn cục
static void _App_Variables_Init() {
    // Cấp phát bộ nhớ cho biến toàn cục chứa trạng thái của robot
    g_p_robot = (robot_object_t *)malloc(sizeof(robot_object_t));
    if (g_p_robot != NULL) {
        // Khởi tạo biến toàn cục chứa trạng thái của robot
        *g_p_robot = Robot_Create(60.0f, 120.0f, 275.0f, -350.0f, -280.0, 137.0f);
        g_p_robot->lock = xSemaphoreCreateMutex(); // Tạo mutex để bảo vệ truy cập vào dữ liệu của robot
    }

    // Cấp chân GPIO cho 3 cánh tay của robot
    Arm_Init(&g_p_robot->_arm_1, ARM_1);
    Arm_Init(&g_p_robot->_arm_2, ARM_2);
    Arm_Init(&g_p_robot->_arm_3, ARM_3);

    // Khởi tạo các Queue với kích thước phù hợp
    g_queue_udp_to_planner = xQueueCreate(1, sizeof(udp_payload_t));  
    g_queue_planner_to_kinematics = xQueueCreate(10, sizeof(point_t)); 
    g_queue_kinematics_to_control = xQueueCreate(5, sizeof(theta_t));
}

// Hàm khởi tạo các task chạy song song trên các core khác nhau
static void _App_Task_Init() {
    // =================================CORE 0=================================
    // Khởi tạo Task lắng nghe UDP từ Python trên Core0
    xTaskCreatePinnedToCore(
        UDP_Receive_Task, // gọi hàm thực thi trên task
        "UDP_Receive", 
        4096, 
        NULL, 
        5, // Ưu tiên cao hơn (5)
        NULL, 
        0);

    xTaskCreatePinnedToCore(
        Robot_Planner_Task, // gọi hàm thực thi trên task
        "Planner", 
        4096, 
        NULL, 
        4, // Ưu tiên thấp hơn (4)
        &g_handle_planner, 
        0);
    // tạm dừng Task Planner để chờ Task Control_Servo đưa các cánh tay về vị trí home
    if (g_handle_planner != NULL) 
        vTaskSuspend(g_handle_planner);
    
    // Khởi tạo Task đánh lừa DNS trên Core0 (OTA)
    xTaskCreatePinnedToCore(
        Wifi_DNS_Server_Task, // gọi hàm thực thi trên task
        "DNS_Server", 
        2048, 
        NULL, 
        3, // Ưu tiên thấp hơn (3)
        NULL, 
        0);


    // =================================CORE 1=================================
    // Khởi tạo task điều khiển động cơ trên Core1
    xTaskCreatePinnedToCore(
        Robot_Motor_Control_Task, // gọi hàm thực thi trên task
        "Control_Servo", 
        4096, 
        NULL, 
        6, 
        NULL, 
        1);

    // Khởi tạo Task xử lý Kinematics trên Core1
    xTaskCreatePinnedToCore(
        Robot_Kinematics_Task, // gọi hàm thực thi trên task
        "Kinematics", 
        8192, 
        NULL, 
        5, 
        &g_handle_kinematics, 
        1);
    // tạm dừng Task Kinematics để chờ Task Control_Servo đưa các cánh tay về vị trí home
    if (g_handle_kinematics != NULL) 
        vTaskSuspend(g_handle_kinematics);
    
}

// Hàm khởi tạo ứng dụng
void App_Init() {
    // Khởi tạo Wifi Access Point với tên "Delta_Robot_v2", mật khẩu "12345678" và cho phép tối đa 1
    Wifi_Init("Delta_Robot_v2", "12345678", 1);

    // Khởi tạo Web Server để lắng nghe kết nối nạp OTA
    OTA_Init_Web_Server();

     // Khởi tạo các biến toàn cục và Queue
    _App_Variables_Init();

    // Khởi tạo các task chạy song song
    _App_Task_Init();
}
