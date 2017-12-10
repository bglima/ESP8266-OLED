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
#include <fonts/fonts.h>
#include <command.h>
#include "image.xbm"
//#include "names.xbm"

/* Change this according to you schematics and display size */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#define SECOND (1000 / portTICK_PERIOD_MS)
#define PROTOCOL SSD1306_PROTO_I2C
#define DISPLAY_ADDR     SSD1306_I2C_ADDR_0
//#define DEFAULT_FONT FONT_FACE_TERMINUS_6X12_ISO8859_1

/* Declare device descriptor */
static const ssd1306_t dev = {
    .protocol = PROTOCOL,
    .i2c_dev.bus = I2C_BUS,
    .i2c_dev.addr = DISPLAY_ADDR,
    .width    = DISPLAY_WIDTH,
    .height   = DISPLAY_HEIGHT
};

/* Attributes */
static bool displayOn;
static bool scrolling;
static bool standByEnabled;
static int timeToStandBy;

/* Font attributes */
static font_info_t *font = NULL; // current font 16, 17, 18
static font_face_t font_face = 0;

/* Local frame buffer */
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
static TimerHandle_t standByTimeHandler = NULL; /* Timer handler */
static TimerHandle_t fontSelectTimeHanlder = NULL;

/* Tasks used */
static void displayTask(void *pvParameters);

/* Internal functions */
void standByTimer(TimerHandle_t h);
void updateFont(TimerHandle_t h);

void displayInit(void);
void setDisplayState(bool state);
void setScrollState(bool state, int timeSelection);
void setStandByTime(int timeToStand);

/* User commands */
static status_t cmdSetAutoStandBy(uint32_t argc, char *argv[]);
static status_t cmdSetDisplay(uint32_t argc, char *argv[]);
static status_t cmdSetScroll(uint32_t argc, char *argv[]);

#endif
