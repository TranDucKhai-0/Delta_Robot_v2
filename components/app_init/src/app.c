#include "app.h"
#include "wifi_app.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void _App_taskInit() {
    
}

void App_init() {
    // Khởi tạo Wifi Access Point với tên "Delta_Robot", mật khẩu "12345678" và cho phép tối đa 1
    Wifi_Init("Delta_Robot_v2", "12345678", 1);

    // Khởi tạo các task chạy song song
    _App_taskInit();
}
