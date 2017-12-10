#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"
#include "command.h"
#include "led_cmd.h"
#include "MPU6050.h"
#include "stdio.h"
#include "display.h"
#include "stdint.h"

/* Queue to handle tap from user */
QueueHandle_t tapQueue;

/* Main task that gets a tap from MPU */
void tapListenerTask(void *pvParameters)
{
    uint8_t code;
    while(1)
    {
        if( xQueueReceive(tapQueue, &code, 50 / portTICK_PERIOD_MS) )
        {
           printf("[SYS] I LISTENED TO A TAP!\n");
        }
    }
}

int user_init( void )
{
    uart_set_baud(0, 115200);
    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);
    tapQueue =  xQueueCreate(1, sizeof(uint8_t));
    xTaskCreate(tapListenerTask, (signed char*) "tapListenerTask", 1024, NULL, 1, NULL);

    cmdInit();
    ledInit();
    mpuInit(&tapQueue);
    vTaskDelay(1 * SECOND);
    displayInit();
    vTaskDelay(1 * SECOND);
    cmdRun("help");

    return 0;
}
