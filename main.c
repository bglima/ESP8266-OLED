#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "i2c.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"

void sender_task(void *p)
{
    int i = 0;
    while(1) {
        printf("Send %i to receiver task!!!\n", i);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void receiver_task(void *p)
{
    int rx_int = 0;
    while(1) {
        printf("Received %i world!\n", rx_int);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


int user_init( void )
{
    uart_set_baud(0, 115200);
    xTaskCreate(my_task, (signed char*) "my_task", 1024, NULL, 1, NULL);
}
