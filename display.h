#ifndef DISPLAY_HEADER
#define DISPLAY_HEADER

#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <string.h>
#include <ssd1306/ssd1306.h>
#include <command.h>

#include "image.xbm"

/* Change this according to you schematics and display size */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#define SECOND (1000 / portTICK_PERIOD_MS)
#define PROTOCOL SSD1306_PROTO_I2C
#define ADDR     SSD1306_I2C_ADDR_0

/* Declare device descriptor */
static const ssd1306_t dev = {
    .protocol = PROTOCOL,
    .i2c_dev.bus = I2C_BUS,
    .i2c_dev.addr = ADDR,
    .width    = DISPLAY_WIDTH,
    .height   = DISPLAY_HEIGHT
};

/* Local frame buffer */
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
static TimerHandle_t scrolTimeHandler = NULL; /* Timer handler */

/* Tasks used */
static void displayTask(void *pvParameters);

/* Internal functions */
void scrollingTimer(TimerHandle_t h);
void displayInit(void);


#endif
