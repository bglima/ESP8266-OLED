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
    ssd1306_set_whole_display_lighting(&dev, false);

    /* Initial settings */
    standByEnabled = true;
    scrolling = false;
    displayOn = true;
    timeToStandBy = 10;

    /* Creating timers */
    fontSelectTimeHanlder = xTimerCreate("font_timer", 1 * SECOND, pdTRUE, NULL, updateFont);
    standByTimeHandler = xTimerCreate("fps_timer", timeToStandBy*SECOND, pdTRUE, NULL, standByTimer);

    if( standByEnabled )
        xTimerStart(standByTimeHandler, 0);
    if ( scrolling )
        ssd1306_start_scroll_hori(&dev, false, 0, 7, FRAME_25);

    showStartMessage();

    /* Adding functions to invoker */
    commandDescriptor_t descriptorSetDisplay = {"display-turn", &cmdSetDisplay, " $display-turn <state>     State can be either 1 or 0\n"};
    commandDescriptor_t descriptorSetScroll = {"display-scroll", &cmdSetScroll, " $display-scroll <state> <time>     A state (0 or 1) and number from 0 to 7\n"};
    commandDescriptor_t descriptorStandBy = {"display-stand", &cmdSetAutoStandBy, " $display-stand <time>     State can be either 1 or 0\n"};
    commandDescriptor_t descriptorFontDemo = {"display-demo", &cmdSetFontDemo, " $display-demo <state>    Shows avaliable fonts. State is 0 or 1\n"};
    cmdInsert(descriptorSetDisplay);
    cmdInsert(descriptorSetScroll);
    cmdInsert(descriptorStandBy);
    cmdInsert(descriptorFontDemo);
}

/************************************
 *          Timed functions
 ************************************/
void updateFont(TimerHandle_t h) {
    if (++font_face >= font_builtin_fonts_count)
        font_face = 0;
    font = font_builtin_fonts[font_face];
    char text[20];
    sprintf(text, "Font n %u ", font_face);
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK);
    ssd1306_draw_string(&dev, buffer, font, 0, 0, text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_load_frame_buffer(&dev, buffer);
}

void standByTimer(TimerHandle_t h)
{
    displayOn = false;
    ssd1306_display_on(&dev, displayOn);
    xTimerStop( h, 0 ) ;
}

/*************************************
 *           Public functions
 *************************************/
bool getDisplayStatus()
{
    return displayOn;
}

bool getScrollingStatus()
{
    return scrolling;
}

void showStartMessage()
{
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK);
    ssd1306_draw_string(&dev, buffer, font_builtin_fonts[23], 0, 0, "Hi, dude!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_draw_string(&dev, buffer, font_builtin_fonts[10], 0, 32, "e-Bruno (c). Rights reserved.", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_load_frame_buffer(&dev, buffer);
}

void showTemperature(float temp)
{
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK);
    font = font_builtin_fonts[16];
    uint16_t textWidth = font_measure_string(font, "Temp.");
    ssd1306_draw_string(&dev, buffer, font, (DISPLAY_WIDTH - textWidth)/2, 1, "Temp.", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    char text[20];
    sprintf(text, "%.2f C", temp);
    font = font_builtin_fonts[23];
    textWidth = font_measure_string(font, text);
    ssd1306_draw_string(&dev, buffer, font, (DISPLAY_WIDTH - textWidth)/2, 35, text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    ssd1306_load_frame_buffer(&dev, buffer);
}


void showLogoImage(uint8_t imageIndex)
{
    unsigned char * bits;
    switch ( imageIndex ) {
    case 0:
        bits = image_bits;
        break;
    case 1:
        bits = names_bits;
        break;
    default:
        bits = image_bits;
        break;
    }
    ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK);
    ssd1306_load_xbm(&dev, bits, buffer);
}

void setDisplayState(bool state)
{
    displayOn = state;
    ssd1306_display_on(&dev, displayOn);
    if ( displayOn && standByEnabled )
        xTimerStart( standByTimeHandler, 0 );
}


void setStandByTime(int8_t timeToStand)
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
        ssd1306_load_frame_buffer(&dev, buffer);
        printf("[SYS] Scrolling stopped successfuly.\n");
    }
}

void setFontDemo(bool state)
{
    if ( state ) {
       xTimerStart(fontSelectTimeHanlder, 0);
    } else {

        xTimerStop(fontSelectTimeHanlder, 0);
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

status_t cmdSetFontDemo(uint32_t argc, char *argv[])
{
    if( argc < 2 ) {
        printf("[ERR] Missing font demo state (0 or 1) to set.\n");
        return FAIL;
    } else {
        bool status = atoi(argv[1]);
        setFontDemo( status );
        if ( status )
            printf("[SYS] Font demo set to ON\n");
        else
            printf("[SYS] Font demo set to OFF\n");
        return OK;
    }
}

