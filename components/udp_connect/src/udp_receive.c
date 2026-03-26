#include "udp_receive.h"
#include "globals.h"
#include "type_data.h"
#include "robot_delta.h"
#include "lwip/sockets.h"
#include "esp_log.h" // Thư viện in Log
#include <string.h>

#define UDP_PORT 1234  // Cổng UDP để Python bắn tới

// Định nghĩa một cái tên (TAG) để dễ lọc Log trên terminal
static const char *TAG = "UDP_RX"; 

void UDP_Receive_Task(void *pvParameters) {  
    char rx_buffer[128];
    point_t target;

    // Dọn sạch rác RAM
    memset(&target, 0, sizeof(point_t));
    
    while (true) {
        // Cấu hình Socket UDP
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);

        // Tạo socket và bind
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

        // Nếu tạo socket thất bại, chờ 0.5s rồi thử lại
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue; // Lặp lại vòng while ngoài
        }

        // kiểm tra lỗi bind, nếu lỗi thì đóng socket và tạo lại từ đầu
        if (bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(500)); // BẮT BUỘC
            continue;
        }
        
        // đặt thời gian chờ nhận dữ liệu để tránh treo task nếu PC không gửi gì
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000; // Cài báo thức 20ms
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        // ====================================================================

        // Chuẩn bị nhận dữ liệu
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);

        while (true) {
            // Hàm này sẽ tự nhả ra sau 20ms nếu PC không gửi gì
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            
            if (len > 0) {
                rx_buffer[len] = 0; // Chốt chuỗi để băm
                
                // === IN LOG CHUỖI THÔ NHẬN ĐƯỢC ===
                // ESP_LOGI(TAG, "Đã nhận chuỗi từ PC: %s", rx_buffer);

                // Bắt đầu băm chuỗi rx_buffer bằng dấu phẩy
                char* ptr = strtok(rx_buffer, ",");
                if (ptr != NULL) {
                    target.mode = atoi(ptr); 
                    
                    ptr = strtok(NULL, ",");
                    if (ptr != NULL) target.x = strtof(ptr, NULL);
                    
                    ptr = strtok(NULL, ",");
                    if (ptr != NULL) target.y = strtof(ptr, NULL);
                    
                    ptr = strtok(NULL, ",");
                    if (ptr != NULL) target.z = strtof(ptr, NULL);

                    // === IN LOG KẾT QUẢ SAU KHI BĂM CHUỖI ===
                    // ESP_LOGI(TAG, "Dữ liệu sau khi giải mã -> Mode: %d | X: %.2f | Y: %.2f | Z: %.2f", 
                    //          target.mode, target.x, target.y, target.z);

                    // Đặt cờ ngắt HOMING
                    if(target.mode == MODE_HOMING) {
                        g_p_robot->should_break_homing = false; // Reset cờ sau khi đã dùng
                    } else {
                        g_p_robot->should_break_homing = true; // Đặt cờ break homing để nếu đang homing thì dừng lại ngay lập tức
                    }

                    xQueueOverwrite(g_queue_udp_to_planner, &target);
                }
            }
            else if (len < 0) {
                // Phân biệt giữa "Timeout" và "Lỗi mạng"
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 11) {
                    // Lỗi này là do hết 20ms mà PC chưa gửi gì tới.
                    continue; 
                } else {
                    // Đây là lỗi nặng (rớt Wi-Fi, socket hỏng)
                    ESP_LOGW(TAG, "Lỗi mạng hoặc mất kết nối, đang khởi tạo lại Socket...");
                    break; // thoát vòng lặp trong để đóng Socket và tạo lại từ đầu
                }
            }
        }
        // Đóng socket
        close(sock);
    }
}