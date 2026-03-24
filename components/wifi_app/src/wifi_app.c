#include "wifi_app.h"

// ========================= Thư viện ESP-IDF hệ thống =========================
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "lwip/sockets.h"
//#include <string.h>

static const char *_TAG = "WIFI_AP_LIB";

// Hàm xử lý sự kiện Wifi (khi có thiết bị kết nối hoặc ngắt kết nối)
static void _Wifi_Event_Handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(_TAG, "Đã có thiết bị kết nối! MAC: " MACSTR, MAC2STR(event->mac));
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(_TAG, "Thiết bị vừa ngắt kết nối! MAC: " MACSTR, MAC2STR(event->mac));
    }
}

// Hàm khởi tạo ESP32 làm trạm phát sóng Wi-Fi (Access Point) (Tên Wifi, pass, số lượng dc phép kết nối)
void Wifi_Init(const char* WIFI_AP_SSID, const char* WIFI_AP_PASS, uint8_t MAX_STA_CONN) {
    // Khởi tạo NVS (Non-Volatile Storage) để lưu cấu hình Wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(_TAG, "Đang khởi động trạm phát Wi-Fi...");

    // Khởi tạo TCP/IP stack và Event Loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // Khởi tạo Wifi với cấu hình mặc định
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Đăng ký hàm xử lý sự kiện (_Wifi_Event_Handler)
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_Wifi_Event_Handler, NULL, NULL));

    // Cấu hình thông số Access Point (SSID, Pass, Max connection)
    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.ap.ssid, WIFI_AP_SSID);
    wifi_config.ap.ssid_len = strlen(WIFI_AP_SSID);
    strcpy((char*)wifi_config.ap.password, WIFI_AP_PASS);
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    if (strlen(WIFI_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // Kích hoạt chế độ AP và bắt đầu phát Wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Cấm Wi-Fi ngủ gật, bắt nó chạy full công suất 100% thời gian!
    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_LOGI(_TAG, "=========================================");
    ESP_LOGI(_TAG, "Wi-Fi Robot đã sẵn sàng!!");
    ESP_LOGI(_TAG, "IP mặc định của Robot: 192.168.4.1");
    ESP_LOGI(_TAG, "=========================================");
}


// Task DNS Server để lừa thiết bị
void Task_DNS_Server(void *pvParameters) {
    char rx_buffer[128];
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53); // 53 là cổng chuẩn của DNS
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while (true) {
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);
        
        // Chờ thiết bị hỏi đường
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 16, 0, (struct sockaddr *)&source_addr, &socklen);
        
        if (len > 12) {
            // Biến câu hỏi của thiết bị thành câu trả lời (DNS Spoofing)
            rx_buffer[2] |= 0x80; // Chuyển cờ thành Response
            rx_buffer[3] |= 0x80; // Cho phép đệ quy
            rx_buffer[6] = 0; rx_buffer[7] = 1; // Số lượng câu trả lời = 1

            // Bơm IP của ESP32 (192.168.4.1) vào đuôi gói tin
            rx_buffer[len++] = 0xC0; rx_buffer[len++] = 0x0C; // Pointer
            rx_buffer[len++] = 0x00; rx_buffer[len++] = 0x01; // Type A
            rx_buffer[len++] = 0x00; rx_buffer[len++] = 0x01; // Class IN
            rx_buffer[len++] = 0x00; rx_buffer[len++] = 0x00; // TTL
            rx_buffer[len++] = 0x00; rx_buffer[len++] = 0x3C; // TTL 60s
            rx_buffer[len++] = 0x00; rx_buffer[len++] = 0x04; // Data Length (4 bytes)
            
            // IP 192.168.4.1
            rx_buffer[len++] = 192;  rx_buffer[len++] = 168; 
            rx_buffer[len++] = 4;    rx_buffer[len++] = 1;   

            // Bắn ngược lại cho điện thoại
            sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
        }
    }
}