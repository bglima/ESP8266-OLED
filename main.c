#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "i2c.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"
#include "command.h"
#include "led_cmd.h"
#include "i2c.h"
#include "MPU6050.h"

int user_init( void )
{
    uart_set_baud(0, 115200);

    cmdInit();
    ledInit();
    i2c_init(SCL_PIN, SDA_PIN);
    mpuInit();

    xTaskCreate(keyReaderTask, (signed char*) "keyManagerTask", 1024, NULL, 1, NULL);
    xTaskCreate(cmdReaderTask, (signed char*) "cmdManagerTask", 1024, NULL, 2, NULL);
    xTaskCreate(cmdHandlerTask, (signed char*) "cmdHandlerTask", 1024, NULL, 3, NULL);
}
