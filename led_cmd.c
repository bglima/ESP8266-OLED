#include "led_cmd.h"

void ledInit()
{
    cmdInsert( {"on",&cmdOn, "$on <gpio number> [ <gpio number>]+     Set gpio to 1\n"} );
}


/*
*
* Imlmementation of possible commands declared .h headers
*
*/
status_t cmdOn(uint32_t argc, char *argv[])
{
   if (argc >= 2) {
       for(int i=1; i<argc; i++) {
           uint8_t gpio_num = atoi(argv[i]);
           gpio_enable(gpio_num, GPIO_OUTPUT);
           gpio_write(gpio_num, true);
           if ( blink_io == gpio_num )
               blink_freq = 0;
           printf("[SYS] On %d\n", gpio_num);
           return OK;
       }
   } else {
       printf("[ERR] Missing gpio number.\n");
       return FAIL;
   }
}

status_t cmdOff(uint32_t argc, char *argv[])
{
   if (argc >= 3) {
       blink_io = atoi(argv[1]);
       blink_freq = atoi(argv[2]);
       gpio_enable(blink_io, GPIO_OUTPUT);
       gpio_toggle(blink_io);
       printf("Blinking %d\n", blink_io);
       return OK;
   } else {
       printf("[ERR] Miissing gpio numbero and/or frequency (Hz)\n");
       return FAIL;
   }
}

status_t cmdBlink(uint32_t argc, char *argv[])
{
   if (argc >= 2) {
       for(int i=1; i<argc; i++) {
           uint8_t gpio_num = atoi(argv[i]);
           gpio_enable(gpio_num, GPIO_OUTPUT);
           gpio_write(gpio_num, false);
           if ( blink_io == gpio_num )
               blink_freq = 0;
           printf("[SYS] Off %d\n", gpio_num);
           return OK;
       }
   } else {
       printf("[SYS] Missing gpio number.\n");
       return FAIL;
   }
}
