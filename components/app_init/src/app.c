#include "app.h"
#include "wifi_app.h"
#include "over_the_air.h"
#include "globals.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static void _App_Variables_Init() {
    // Khởi tạo các Queue với kích thước phù hợp
    g_queue_udp_to_planner = xQueueCreate(1, sizeof(point_t));  
    g_queue_planner_to_core1 = xQueueCreate(10, sizeof(point_t)); 
}

static void _App_Task_Init() {
    // =================================CORE 0=================================
    // Khởi tạo Task đánh lừa DNS trên Core0 (OTA)
    xTaskCreatePinnedToCore(
        Wifi_DNS_Server, // gọi hàm thực thi trên task
        "DNS_Server", 
        2048, 
        NULL, 
        3, // Ưu tiên thấp hơn (3)
        NULL, 
        0);
    // =================================CORE 1=================================

}

void App_init() {
    // Khởi tạo Wifi Access Point với tên "Delta_Robot_v2", mật khẩu "12345678" và cho phép tối đa 1
    Wifi_Init("Delta_Robot_v2", "12345678", 1);

    // Khởi tạo Web Server để lắng nghe kết nối nạp OTA
    OTA_Init_Web_Server();

    _App_Variables_Init(); // Khởi tạo các biến toàn cục và Queue

    // Khởi tạo các task chạy song song
    _App_Task_Init();
}
