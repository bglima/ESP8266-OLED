#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "esp8266.h"
#include "task.h"
#include "queue.h"
#include "esp/uart.h"
#include "command.h"
#include "led_cmd.h"
#include "MPU6050.h"
#include "stdio.h"
#include "display.h"
#include "stdint.h"

/* Variables used in main */
QueueHandle_t tapQueue; /* Stores tap information sent from MPU */
uint8_t state; /*
                * State changes whenever a double tap is pressed. Single tap just wake the screen.
                *
                * 0 = initial state (show initial message)
                * 1 = show image 1
                * 2 = show image 2
                * 3 = get and show temperature from MPU
                *
                */
void tapStateMachine(void *pvParameters); /* Task prototype */
void updateState(uint8_t code);

/* Init procedures */
int user_init( void )
{
    uart_set_baud(0, 115200);
    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);

    state = 0;
    tapQueue =  xQueueCreate(1, sizeof(uint8_t));
    xTaskCreate(tapStateMachine, (signed char*) "tapListenerTask", 256, NULL, 1, NULL);

    cmdInit();
    ledInit();
    mpuInit(&tapQueue);
    vTaskDelay(1 * SECOND);
    displayInit();
    vTaskDelay(1 * SECOND);
    cmdRun("help");

    return 0;
}

/* Main task that gets a tap from MPU */
void tapStateMachine(void *pvParameters)
{
    uint8_t code;
    while(1)
    {
        while ( !xQueueReceive(tapQueue, &code, 0 / portTICK_PERIOD_MS) )
            taskYIELD();
        updateState( code );
    }
}

/* Process current state and return next */
void updateState(uint8_t code) {
    /* Double tapped */
    if ( code ) {
        ++state;
        if ( state > 3 )
            state = 0;
        return;
    }

    /* Single tapped */
    switch( state ) {
    case 0:                         /* Start message */
        if ( getDisplayStatus() ) {
            setScrollState( !getScrollingStatus(), 4);
            setDisplayState( true );
        } else {
            showStartMessage();
            setDisplayState( true );
        }

        break;
    case 1:
        showLogoImage(0);
        break;
    case 2:
        showLogoImage(1);
        break;
    case 3:
        showTemperature( mpuGetTemp() );
        break;
    default:
        break;
    }



}


