#pragma once

#include <stdbool.h>
#include <stdint.h>

// Cấu trúc dữ liệu cho gripper
// gripper_object_t name = { .GPIO_INDX = NUM_GPIO, .TIME_DELAY_MS = MS};
typedef struct gripper_delta{
    uint8_t GPIO_INDX; // Chân GPIO điều khiển gripper 
    uint16_t TIME_DELAY_MS; // Thời gian delay cần để gắp được vật khi bật/tắt gripper
    bool is_on; // biến trạng thái của gripper 
} gripper_object_t;

// Hàm khởi tạo gripper, thiết lập trạng thái ban đầu cho gripper nhả (false nhả hoặc true nhả) tùy vào thiết kế cơ khí 
void Gripper_Init(gripper_object_t *p_gripper, bool initial_state);

void Gripper_Default_State(gripper_object_t *p_gripper, bool default_state);

// Hàm viết giá trị GPIO để bật/tắt gripper
void Gripper_Change(gripper_object_t *p_gripper); // Hàm bật gripper
