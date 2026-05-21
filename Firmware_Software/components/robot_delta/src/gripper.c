#include "gripper.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


static void _Gripper_Write_GPIO(gripper_object_t *p_gripper, bool state)
{
    if (p_gripper == NULL) return;


    // Ghi giá trị GPIO cho gripper
    esp_err_t err = gpio_set_level(p_gripper->GPIO_INDX, state);
    if (err == ESP_OK)
    {
        p_gripper->is_on = state; 
    }
}

void Gripper_Default_State(gripper_object_t *p_gripper, bool default_state)
{
    if (p_gripper == NULL) return;

    _Gripper_Write_GPIO(p_gripper, default_state); // Đặt trạng thái mặc định cho gripper
}

void Gripper_Init(gripper_object_t *p_gripper, bool initial_state)
{
    // Reset chân GPIO trước khi cấu hình để đảm bảo nó ở trạng thái mặc định
    gpio_reset_pin(p_gripper->GPIO_INDX);
    // Cấu hình chân GPIO làm OUTPUT
    gpio_set_direction(p_gripper->GPIO_INDX, GPIO_MODE_OUTPUT);

    // Thiết lập trạng thái ban đầu cho gripper nhả
    Gripper_Default_State(p_gripper, initial_state);
}


void Gripper_Change(gripper_object_t *p_gripper)
{
    if (p_gripper == NULL) return;
    
    _Gripper_Write_GPIO(p_gripper, !p_gripper->is_on); // Đảo trạng thái hiện tại của gripper
    vTaskDelay(pdMS_TO_TICKS(p_gripper->TIME_DELAY_MS)); // delay để đảm bảo gripper đã kịp gắp/thả được vật trước khi tiếp tục thực hiện các bước tiếp theo
}
