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
#define MAX_NUM_OF_CMD 10

/*
 *
 * Defining global types
 *
 */
typedef enum {
    OK,
    FAIL
} status_t;

//typedef enum {
//    CMD_ON,
//    CMD_OFF,
//    CMD_BLINK,
//    CMD_SLEEP,
//    CMD_HELP
//} __attribute__((__packed__)) command_t;

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
//static status_t cmdOn(uint32_t argc, char *argv[]);
//static status_t cmdOff(uint32_t argc, char *argv[]);
//static status_t cmdBlink(uint32_t argc, char *argv[]);
static status_t cmdSleep(uint32_t argc, char *argv[]);
static status_t cmdHelp(uint32_t argc, char *argv[]);

/*
 *  Attributes
 */
uint8_t currentNumOfCommands = 0;
static commandDescriptor_t invoker[MAX_NUM_OF_CMD];

//static commandDescriptor_t invoker[NUM_OF_CMD] = {
//    {"on", &cmdOn, "  $on <gpio number> [ <gpio number>]+     Set gpio to 1\n"},
//    {"off", &cmdOff, "  $off <gpio number> [ <gpio number>]+    Set gpio to 0\n"},
//    {"blink", &cmdBlink, "  $blink <gpio number> <frequency>    Starts blining gpio\n"},
//    {"sleep", &cmdSleep, "  $sleep     Take a nap for two seconds\n"},
//    {"help", &cmdHelp, "  $help     Show all avaliable commands\n"}
//};

//static uint8_t blink_io = 2;    /* Internal led is GPIO = 2 */
//static uint8_t blink_freq = 0;  /* Initialize with blink off */


#endif
