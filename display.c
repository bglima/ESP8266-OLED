#include <display.h>


/***************************************
 *    Main configuration functions
 ***************************************/
void displayInit(void)
{
    printf("[SYS] Initiating SSD1603...\n");
    ssd1306_init(&dev);
    ssd1306_set_whole_display_lighting(&dev, true);
    vTaskDelay(SECOND);

    /* Initial settings */
    standByEnabled = false;
    scrolling = true;
    displayOn = true;
    timeToStandBy = 5;

    /* Scrolling timer */
    standByTimeHandler = xTimerCreate("fps_timer", timeToStandBy*SECOND, pdTRUE, NULL, standByTimer);
    if( standByEnabled )
        xTimerStart(standByTimeHandler, 0);

    for( int i = 0; i < font_builtin_fonts_count; ++i ) {
        printf("Font %d\n", i);
    }

    font = font_builtin_fonts[0];
    /* Load xbm images */
    ssd1306_set_whole_display_lighting(&dev, false);
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK);
    ssd1306_draw_string(&dev, buffer, font, 0, 0, "Hello, esp-open-rtos!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
//    ssd1306_load_frame_buffer(&dev, buffer);
    ssd1306_draw_string(&dev, buffer, &font_builtin_fonts[0], 0, 45, "Btuno Lima", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_load_frame_buffer(&dev, buffer);


//    ssd1306_load_xbm(&dev, image_bits, buffer);
    //uint8_t w = ssd1306_measure_string(0, "www.ba0sh1.com");
    //ssd1306_draw_string(&dev, 0, (ssd1306_get_width(0) - w) / 2, 12, "www.ba0sh1.com", 0x01, 0x00);
//    ssd1306_set_whole_display_lighting(&dev, true);
//    ssd1306_set_whole_display_lighting(&dev, false);
    
    //ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT/2, OLED_COLOR_BLACK);
//    ssd1306_draw_string(&dev, buffer, font, 0, 0, "Hello, esp-open-rtos!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
//    ssd1306_draw_string(&dev, buffer, font_builtin_fonts[0], 2, 12, "Hello, esp-open-rtos!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    fontSelectTimeHanlder = xTimerCreate("font_timer", 1 * SECOND, pdTRUE, NULL, updateFont);
    xTimerStart(fontSelectTimeHanlder, 0);

    /* Adding functions to invoker */
    commandDescriptor_t descriptorSetDisplay = {"display-turn", &cmdSetDisplay, " $display-turn <state>     State can be either 1 or 0\n"};
    commandDescriptor_t descriptorSetScroll = {"display-scroll", &cmdSetScroll, " $display-scroll <state> <time>     A state (0 or 1) and number from 0 to 7\n"};
    commandDescriptor_t descriptorStandBy = {"display-stand", &cmdSetAutoStandBy, " $display-stand <time>     State can be either 1 or 0\n"};
    cmdInsert(descriptorSetDisplay);
    cmdInsert(descriptorSetScroll);
    cmdInsert(descriptorStandBy);
}



//static void displayTask(void *pvParameters)
//{

//    bool fwd = true;
//    while (1) {
//        vTaskDelay(2*SECOND);   /* Flip direction each 2 seconds */
//        //printf("[SYS] %s: still alive, flipping!\n", __FUNCTION__);
//        ssd1306_set_scan_direction_fwd(&dev, fwd);
//        //fwd = !fwd;
//    }
//}

void updateFont(TimerHandle_t h) {
        if (++font_face >= font_builtin_fonts_count)
            font_face = 0;
        font = font_builtin_fonts[font_face];

    printf("Selected builtin font %d\n", font_face);
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT/2, OLED_COLOR_BLACK);
    ssd1306_draw_string(&dev, buffer, font, 0, 0, "Hello, esp-open-rtos!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_load_frame_buffer(&dev, buffer);
}

void standByTimer(TimerHandle_t h)
{
    displayOn = false;
    ssd1306_display_on(&dev, displayOn);
    xTimerStop( h, 0 ) ;
}

/*************************************
 *           Internal functions
 *************************************/
void setDisplayState(bool state)
{
    displayOn = state;
    ssd1306_display_on(&dev, displayOn);
    if ( displayOn && standByEnabled )
        xTimerStart( standByTimeHandler, 0 );
}


void setStandByTime(int timeToStand)
{
    if ( !timeToStand || timeToStand < 0) {
        standByEnabled = false;
        timeToStandBy = 0;
    } else {
        standByEnabled = true;
        timeToStandBy = timeToStand;
        xTimerChangePeriod( standByTimeHandler, timeToStandBy * SECOND, 0);
    }
}

void setScrollState(bool state, int timeSel)
{
    scrolling = state;
    if( scrolling ) {
        if (timeSel < 0 || timeSel > 7) {
            printf("[SYS] Time sel not valid. Using FRAME_25\n");
            ssd1306_start_scroll_hori(&dev, false, 0, 7, FRAME_25);
        } else {
            ssd1306_start_scroll_hori(&dev, false, 0, 7, timeSel);
            printf("[SYS] Scrolling set successfuly.\n");
        }
    } else {
        ssd1306_stop_scroll(&dev);
        printf("[SYS] Scrolling stopped successfuly.\n");
    }
}



/****************************************
 *    User general purpose functions
 ****************************************/
status_t cmdSetDisplay(uint32_t argc, char *argv[])
{
    if( argc < 2 ) {
        printf("[ERR] Missing display state (0 or 1) to set.\n");
        return FAIL;
    } else {
        setDisplayState( atoi(argv[1]) );
        printf("[SYS] Display set to %s\n", (displayOn)? "On" : "Off");
        return OK;
    }
}

status_t cmdSetScroll(uint32_t argc, char *argv[])
{
    if( argc < 2 ) {
        printf("[ERR] Missing scroll state (0 or 1) to set.\n");
        return FAIL;
    } else if ( atoi(argv[1]) && argc < 3 ) {
        printf("[ERR] Missing time selection (between 0 and 7).\n");
        return FAIL;
    } else {
        bool newState = atoi(argv[1]);
        if( !newState ) {
            setScrollState(newState, 0);
            return OK;
        } else {
            int newTime = atoi(argv[2]);
            setScrollState(newState, newTime);
            return OK;
        }
    }
}

status_t cmdSetAutoStandBy(uint32_t argc, char *argv[])
{
    if( argc < 2 ) {
        printf("[ERR] Missing standby state (0 or 1) to set.\n");
        return FAIL;
    } else {
        int timeSel = argv[1];
        setStandByTime( atoi(argv[1]) );
        printf("[SYS] Auto standBy mode set to %s\n", (displayOn)? "On" : "Off");
        return OK;
    }
}
