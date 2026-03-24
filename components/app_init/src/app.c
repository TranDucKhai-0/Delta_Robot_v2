#include "app.h"
#include "wifi_app.h"
#include "over_the_air.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void _App_taskInit() {
    // =================================CORE 0=================================
    // Khởi tạo Task đánh lừa DNS trên Core0 (OTA)
    xTaskCreatePinnedToCore(
        Wifi_DNS_Server, 
        "DNS_Server", 
        2048, 
        NULL, 
        3, // Ưu tiên thấp hơn (3)
        NULL, 
        0);
    // =================================CORE 1=================================

}

void App_init() {
    // Khởi tạo Wifi Access Point với tên "Delta_Robot", mật khẩu "12345678" và cho phép tối đa 1
    Wifi_Init("Delta_Robot_v2", "12345678", 1);

    // Khởi tạo Web Server để lắng nghe kết nối nạp OTA
    OTA_Init_Web_Server();

    // Khởi tạo các task chạy song song
    _App_taskInit();
}
