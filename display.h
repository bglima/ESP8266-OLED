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
#include "names.xbm"

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
static const font_info_t *font = NULL;
static font_face_t font_face = 0;

/* Local frame buffer */
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

/* Timer handlers */
static TimerHandle_t standByTimeHandler = NULL;
static TimerHandle_t fontSelectTimeHanlder = NULL;

/* Tasks used */
static void displayTask(void *pvParameters);

/* Internal timed functions */
static void standByTimer(TimerHandle_t h);
static void updateFont(TimerHandle_t h);

/* Public funtions */
bool getDisplayStatus();
bool getScrollingStatus();
void displayInit(void);
void setFontDemo(bool state);
void setDisplayState(bool state);
void setScrollState(bool state, int timeSelection);
void setStandByTime(int8_t timeToStand);
void showStartMessage();
void showLogoImage(uint8_t imageIndex);
void showTemperature( float temp );

/* User commands */
static status_t cmdSetAutoStandBy(uint32_t argc, char *argv[]);
static status_t cmdSetDisplay(uint32_t argc, char *argv[]);
static status_t cmdSetScroll(uint32_t argc, char *argv[]);
static status_t cmdSetFontDemo(uint32_t argc, char *argv[]);

#endif
