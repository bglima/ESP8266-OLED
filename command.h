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
#define MAX_NUM_OF_CMD 20

#define SDA_PIN (0) // D3
#define SCL_PIN (2) // D4
#define I2C_BUS (0)

/*
 *
 * Defining global types
 *
 */
typedef enum {
    OK,
    FAIL
} status_t;

typedef status_t (*function_t)(uint32_t argc, char *argv[]);

typedef struct {
    char * cmdString;
    function_t receiver;
    char * cmdHelp;
} commandDescriptor_t;

/*
 *
 * Queues used to task intercommunication
 *
 */
static QueueHandle_t keyQueue;
static QueueHandle_t cmdQueue;

/*
 *
 * Task prototypes
 *
 */
void cmdHandlerTask(void *pvParameters);
void cmdReaderTask(void *pvParameters);
void keyReaderTask(void *pvParameters);

/*
 *
 * Function prototypes
 *
 */
void cmdInit();
void cmdRun( char* cmd );
bool cmdInsert( commandDescriptor_t newCmd );

/*
 *
 * Command prototypes
 *
 */
static status_t cmdSleep(uint32_t argc, char *argv[]);
static status_t cmdHelp(uint32_t argc, char *argv[]);

/*
 *  Attributes
 */
static uint8_t currentNumOfCommands = 0;
static commandDescriptor_t invoker[MAX_NUM_OF_CMD];


#endif
