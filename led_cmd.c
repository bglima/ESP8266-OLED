#include "led_cmd.h"

void ledInit()
{
    commandDescriptor_t descriptorOn = {"on",&cmdOn, " $on <gpio number> [ <gpio number>]+     Set gpio to 1\n"};
    commandDescriptor_t descriptorOff =  {"off", &cmdOff, " $off <gpio number> [ <gpio number>]+    Set gpio to 0\n"};
    commandDescriptor_t descriptorBlink =  {"blink", &cmdBlink, " $blink <gpio number> <frequency>    Starts blining gpio\n"};

    cmdInsert( descriptorOn );
    cmdInsert( descriptorOff );
    cmdInsert( descriptorBlink );
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

status_t cmdBlink(uint32_t argc, char *argv[])
{
   if (argc >= 3) {
       blink_io = atoi(argv[1]);
       blink_freq = atoi(argv[2]);
       gpio_enable(blink_io, GPIO_OUTPUT);
       gpio_toggle(blink_io);
       printf("[SYS] Blinking gpio %d\n", blink_io);
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
           if ( blink_io == gpio_num )
               blink_freq = 0;
           printf("[SYS] Off %d\n", gpio_num);
           return OK;
       }
   } else {
       printf("[ERR] Missing gpio number.\n");
       return FAIL;
   }
}
