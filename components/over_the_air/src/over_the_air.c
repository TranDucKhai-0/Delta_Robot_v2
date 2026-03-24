#include "over_the_air.h"

#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "driver/gpio.h"

#define _BLINK_GPIO GPIO_NUM_2 // Chân 2 Led

static const char *P_TAG = "OTA_UPDATE";

// Giao diện Web + JavaScript để nạp code OTA qua trình duyệt
static const char* P_INDEX_HTML = 
    "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Robot Delta OTA</title></head>"
    "<body style='text-align:center; font-family:Arial; margin-top:50px;'>"
    "<h2>UPDATE FIRMWARE (OTA)</h2>"
    "<input type='file' id='fileInput' accept='.bin' style='margin:20px;'><br>"
    "<button onclick='uploadFile()' style='padding:10px 20px; background:#4CAF50; color:white; border:none; border-radius:5px; cursor:pointer;'>Upload</button>"
    "<p id='status' style='font-weight:bold; margin-top:20px;'></p>"
    "<script>"
    "function uploadFile() {"
        "var fileInput = document.getElementById('fileInput');"
        "var status = document.getElementById('status');"
        "if (fileInput.files.length === 0) { status.innerHTML = 'Chưa chọn file firmware.bin'; status.style.color = 'red'; return; }"
        "var file = fileInput.files[0];"
        "status.innerHTML = 'Đang nạp code... Vui lòng không đóng trang này!'; status.style.color = 'blue';"
        "fetch('/update', { method: 'POST', body: file }).then(response => {"
            "if (response.ok) { status.innerHTML = 'UPLOAD THÀNH CÔNG! Robot đang khởi động lại...'; status.style.color = 'green'; }"
            "else { status.innerHTML = 'Lỗi nạp code!'; status.style.color = 'red'; }"
        "}).catch(err => { status.innerHTML = 'Mất kết nối mạng!'; status.style.color = 'red'; });"
    "}"
    "</script></body></html>";

// Xử lý khi truy cập vào IP (192.168.4.1)
static esp_err_t _Index_Get_Handler(httpd_req_t *req) {
    httpd_resp_send(req, P_INDEX_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Xử lý khi bấm nút "Update Fireware"
static esp_err_t _Update_Post_Handler(httpd_req_t *req) {
    char buf[1024];
    int received = 0;
    int remaining = req->content_len;
    bool led_state = false;

    // Tìm vùng App (Partition) đang trống để ghi đè vào
    const esp_partition_t *p_update_partition = esp_ota_get_next_update_partition(NULL);
    esp_ota_handle_t update_handle = 0;

    if (p_update_partition == NULL) {
        ESP_LOGE(P_TAG, "Không tìm thấy phân vùng OTA hợp lệ!");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(P_TAG, "Bắt đầu nạp code vào phân vùng: %s", p_update_partition->label);
    esp_err_t err = esp_ota_begin(p_update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(P_TAG, "Lỗi khởi tạo OTA (esp_ota_begin failed)");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Vòng lặp nhận từng cục data (chunk) từ trình duyệt và ghi vào Flash
    while (remaining > 0) {
        if ((received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) continue; // Bị trễ xíu thì đợi tiếp
            ESP_LOGE(P_TAG, "Lỗi nhận data từ mạng!");
            esp_ota_end(update_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        // Ghi cục data vừa nhận xuống Flash
        err = esp_ota_write(update_handle, (const void *)buf, received);
        if (err != ESP_OK) {
            ESP_LOGE(P_TAG, "Lỗi ghi Flash (esp_ota_write failed)");
            esp_ota_end(update_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        remaining -= received;

        // ================= HIỆU ỨNG ÁNH SÁNG =================
        // Cứ mỗi cục data ghi thành công, đảo trạng thái LED 1 lần
        led_state = !led_state;
        gpio_set_level(_BLINK_GPIO, led_state);
        // =====================================================
    }

    // Kết thúc phiên nạp code
    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(P_TAG, "Lỗi kết thúc OTA (esp_ota_end failed)");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Đổi công tắc khởi động sang vùng App mới vừa nạp xong
    err = esp_ota_set_boot_partition(p_update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(P_TAG, "Lỗi cài đặt phân vùng khởi động (esp_ota_set_boot_partition failed)");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    ESP_LOGI(P_TAG, "Nạp code thành công! Robot sẽ tự khởi động lại...");
    httpd_resp_sendstr(req, "OK");
    
    // Báo nạp thành công sáng 2s
    gpio_set_level(_BLINK_GPIO, 1);

    // Ngủ 2 giây cho web kịp phản hồi rồi reset chip
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();

    return ESP_OK;
}

// Hàm đánh chặn (Captive Portal Handler)
static esp_err_t _Captive_Portal_Handler(httpd_req_t *req, httpd_err_code_t err) {
    // Điều hướng (Redirect) 100% người đi lạc về trang chủ của Robot
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void OTA_Init_Web_Server() {
    // Cấu hình chân LED làm Output
    gpio_reset_pin(_BLINK_GPIO);
    gpio_set_direction(_BLINK_GPIO, GPIO_MODE_OUTPUT); // cấu hình chân LED làm Output
    gpio_set_level(_BLINK_GPIO, 0); // Tắt LED lúc bình thường

    // Cấu hình Web Server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80; // Port mặc định của Web

    config.stack_size = 8192;        
    config.recv_wait_timeout = 10;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        // Đăng ký đường dẫn trang chủ GET "/"
        httpd_uri_t uri_get = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = _Index_Get_Handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get);

        // Đăng ký đường dẫn xử lý upload POST "/update"
        httpd_uri_t uri_post = {
            .uri      = "/update",
            .method   = HTTP_POST,
            .handler  = _Update_Post_Handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_post);
        
        ESP_LOGI(P_TAG, "OTA Web Server đã chạy! Truy cập http://192.168.4.1 để nạp code.");
        // Đăng ký đánh chặn mọi URL không hợp lệ (Lỗi 404)
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, _Captive_Portal_Handler);
    } else {
        ESP_LOGE(P_TAG, "Lỗi không thể khởi động Web Server!");
    }
}