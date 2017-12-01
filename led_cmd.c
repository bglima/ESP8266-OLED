#include "led_cmd.h"

/*
 * Insert led related commands to invoker.
 */
void ledInit()
{
    commandDescriptor_t descriptorOn = {"on",&cmdOn, " $on <gpio number> [ <gpio number>]+     Set gpio to 1\n"};
    commandDescriptor_t descriptorOff =  {"off", &cmdOff, " $off <gpio number> [ <gpio number>]+    Set gpio to 0\n"};
    commandDescriptor_t descriptorBlink =  {"blink", &cmdBlink, " $blink <gpio number> <frequency>    Starts blining gpio\n"};

    cmdInsert( descriptorOn );
    cmdInsert( descriptorOff );
    cmdInsert( descriptorBlink );

    xTaskCreate(blinkLedTask, "blinkLedTask", 256, NULL, 1, NULL);
}

/*
*
* Imlmementation of commands declared .h headers
*
*/
status_t cmdOn(uint32_t argc, char *argv[])
{
   if (argc >= 2) {
       for(int i=1; i<argc; i++) {
           uint8_t gpio_num = atoi(argv[i]);
           gpio_enable(gpio_num, GPIO_OUTPUT);
           gpio_write(gpio_num, true);
           if ( blinkGpio == gpio_num )
               blinkFreq = 0;
           printf("[SYS] On %d\n", gpio_num);
           return OK;
       }
   } else {
       printf("[ERR] Missing gpio number.\n");
       return FAIL;
   }
}

status_t cmdBlink(uint32_t argc, char *argv[])
{
   if (argc >= 3) {
       blinkGpio = atoi(argv[1]);
       blinkFreq = atoi(argv[2]);
       gpio_enable(blinkGpio, GPIO_OUTPUT);
       gpio_toggle(blinkGpio);
       printf("[SYS] Blinking gpio %d\n", blinkGpio);
       return OK;
   } else {
       printf("[ERR] Miissing gpio numbero and/or frequency (Hz)\n");
       return FAIL;
   }
}

status_t cmdOff(uint32_t argc, char *argv[])
{
   if (argc >= 2) {
       for(int i=1; i<argc; i++) {
           uint8_t gpio_num = atoi(argv[i]);
           gpio_enable(gpio_num, GPIO_OUTPUT);
           gpio_write(gpio_num, false);
           if ( blinkGpio == gpio_num )
               blinkFreq = 0;
           printf("[SYS] Off %d\n", gpio_num);
           return OK;
       }
   } else {
       printf("[ERR] Missing gpio number.\n");
       return FAIL;
   }
}

/*
 * Task responsible for blink the led. Instantiated at ledInit()
 */
void blinkLedTask(void *pvParameters)
{
    while( 1 ) {
        if ( blinkFreq > 0 ) {
            gpio_toggle(blinkGpio);
            vTaskDelay( (1000 / blinkFreq) / portTICK_PERIOD_MS);
         } else {
            vTaskDelay( 1000 / portTICK_PERIOD_MS);
        }
    }
}
