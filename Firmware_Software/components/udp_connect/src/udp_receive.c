#include "udp_receive.h"
#include "globals.h"
#include "type_data.h"
#include "robot_delta.h"
#include "lwip/sockets.h"
#include "esp_log.h" 
#include <string.h>
#include <stdlib.h>

#define UDP_PORT 1234  

static const char *TAG = "UDP_RX"; 

void UDP_Receive_Task(void *pvParameters) {  
    char rx_buffer[128];
    udp_payload_t payload; // Sử dụng struct mới

    while (true) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);

        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue; 
        }

        if (bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(500)); 
            continue;
        }
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000; 
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);

        while (true) {
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            
            if (len > 0) {
                rx_buffer[len] = 0; 
                memset(&payload, 0, sizeof(udp_payload_t)); // Xóa sạch hộp chứa

                char* ptr = strtok(rx_buffer, ",");
                if (ptr != NULL) {
                    int current_mode = atoi(ptr);
                    payload.target.mode = current_mode; // Luôn gán mode vào target để Planner đọc được nhánh if
                    
                    // Xử lý CẤU TRÚC 1 (Mode 0, 1, 2)
                    if (current_mode == 0 || current_mode == 1 || current_mode == 2) {
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.target.x = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.target.y = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.target.z = strtof(ptr, NULL);
                    }
                    // Xử lý CẤU TRÚC 2 (Mode 3)
                    else if (current_mode == 3) {
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.pick.x = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.pick.y = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.pick.z = strtof(ptr, NULL);
                        
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.place.x = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.place.y = strtof(ptr, NULL);
                        if ((ptr = strtok(NULL, ",")) != NULL) payload.place.z = strtof(ptr, NULL);
                    }

                    // Xử lý cờ ngắt HOMING giữ nguyên như cũ
                    xSemaphoreTake(g_p_robot->lock, portMAX_DELAY); 
                    if(current_mode == 0) {
                        g_p_robot->should_break_homing = false; 
                    } else {
                        g_p_robot->should_break_homing = true; 
                    }
                    xSemaphoreGive(g_p_robot->lock); 

                    // Bắn trọn gói payload sang Task Planner
                    xQueueOverwrite(g_queue_udp_to_planner, &payload);
                }
            }
            else if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 11) {
                    continue; 
                } else {
                    break; 
                }
            }
        }
        close(sock);
    }
}