#ifndef LED_COMMAND_HEADER
#define LED_COMMAND_HEADER

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <esp8266.h>
#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "command.h"

static status_t cmdOn(uint32_t argc, char *argv[]);
static status_t cmdOff(uint32_t argc, char *argv[]);
static status_t cmdBlink(uint32_t argc, char *argv[]);

static uint8_t blink_io = 2;    /* Internal led is GPIO = 2 */
static uint8_t blink_freq = 0;  /* Initialize with blink off */

/*
 * Function prototypes
 */
void ledInit();

#endif
