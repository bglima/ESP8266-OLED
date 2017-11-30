#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "i2c.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"

#define MAX_LINE_SIZE 81

/*
 * Queues used to task intercommunication
 */
static QueueHandle_t tsqueue;

void receiver_task(void *pvParameters)
{
     char rx_str[81];
    while(1) {
        if( xQueueReceive(tsqueue, rx_str, 1000 / portTICK_PERIOD_MS) )
                  printf("Received: %s\n", rx_str);
    }
}

void keyboard_monitor_task(void *pvParameters)
{
    char ch;
    char cmd[MAX_LINE_SIZE];   // Max string size is 80. Last char must be \n or \r
    int i = 0;
    printf("\n\n\nWelcome to gpiomon. Type 'help<enter>' for, well, help\n");
    printf("%% ");
    fflush(stdout); // stdout is line buffered
    while(1) {
        if (read(0, (void*)&ch, 1)) { // 0 is stdin
            printf("%c", ch);
            fflush(stdout);
            if (ch == '\n' || ch == '\r') {
                cmd[i] = 0;
                i = 0;
                printf("\n");

                // Allow handler to execute its
//                receiver_handle((char*) cmd);   // Handle cmd line that was read
                if ( xQueueSend(tsqueue, cmd, 300 / portTICK_PERIOD_MS) )
                    printf("Sending command to queue!\n");

                printf("%% ");
                fflush(stdout);
            } else {
                if (i < sizeof(cmd)) cmd[i++] = ch;
            }
        } else {
            printf("You will never see this print as read(...) is blocking\n");
        }

    }
}

int user_init( void )
{
    uart_set_baud(0, 115200);

    /*
     * We need a queue of the size of max command line
     */
    tsqueue = xQueueCreate(5, sizeof(char)*MAX_LINE_SIZE);
    xTaskCreate(keyboard_monitor_task, (signed char*) "key_mon", 1024, NULL, 1, NULL);
    xTaskCreate(receiver_task, (signed char*) "receiver_task", 1024, NULL, 1, NULL);

}
