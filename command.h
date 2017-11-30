#ifndef COMMAND_HEADER
#define COMMAND_HEADER

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <esp8266.h>
#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define MAX_LINE_SIZE 81
#define MAX_ARGC 10

/* Queues used to task intercommunication */
static QueueHandle_t keyQueue;
static QueueHandle_t cmdQueue;

/* Task prototypes */
void cmdHandlerTask(void *pvParameters);
void cmdReaderTask(void *pvParameters);
void keyReaderTask(void *pvParameters);

/* Functions */
void cmdInit();

#endif
