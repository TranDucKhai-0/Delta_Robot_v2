#include "udp_receive.h"
#include "globals.h"

#include "lwip/sockets.h"

#define UDP_PORT 1234  // Cổng UDP để Python bắn tới

void UDP_Receive_Task(void *pvParameters) {  
    char rx_buffer[128];
    point_t target;
    
    while (true) {
        // Cấu hình Socket UDP
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);

        // Tạo socket và bind
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        // Nếu tạo socket thất bại, chờ 1s rồi thử lại
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

                    xQueueOverwrite(g_queue_udp_to_planner, &target);
                }
            }
            else if (len < 0) {
                // Cực kỳ quan trọng: Phân biệt giữa "Timeout" và "Lỗi mạng"
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 11) {
                    // Lỗi này là do hết 20ms mà PC chưa gửi gì tới.
                    continue; 
                } else {
                    // Đây là lỗi nặng (rớt Wi-Fi, socket hỏng)
                    break; // thoast vòng lặp trong để đóng Socket và tạo lại từ đầu
                }
            }
        }
        // Đóng socket
        close(sock);
    }
}