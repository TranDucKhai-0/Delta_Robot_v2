#pragma once
#include <stdio.h>

// Hàm khởi tạo ESP32 làm trạm phát sóng Wi-Fi (Access Point) (Tên Wifi, pass, số lượng dc phép kết nối)
void Wifi_Init(const char* WIFI_AP_SSID, const char* WIFI_AP_PASS, uint8_t MAX_STA_CONN);

// Task DNS Server để lừa thiết bị
void Wifi_DNS_Server(void *pvParameters);