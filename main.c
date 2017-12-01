#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"
#include "command.h"
#include "led_cmd.h"
#include "MPU6050.h"
#include "ssd1306.h"
#include "stdio.h"
#include <i2c/i2c.h>

int user_init( void )
{
    uart_set_baud(0, 115200);

    cmdInit();
    ledInit();
    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);
    mpuInit();


    cmdRun("mpu-check");
}
