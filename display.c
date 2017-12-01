#include <display.h>

static void displayTask(void *pvParameters)
{
    printf("%s: Started user interface task\n", __FUNCTION__);
    vTaskDelay(1000/portTICK_PERIOD_MS);


    if (ssd1306_load_xbm(&dev, image_bits, buffer))
        goto error_loop;

    ssd1306_set_whole_display_lighting(&dev, false);
    bool fwd = false;
    while (1) {
        vTaskDelay(2*SECOND);
        printf("[SYS] %s: still alive, flipping!\n", __FUNCTION__);
        ssd1306_set_scan_direction_fwd(&dev, fwd);
        fwd = !fwd;
    }

error_loop:
    printf("[ERR] %s: error while loading framebuffer into SSD1306\n", __func__);
    for(;;){
        vTaskDelay(2*SECOND);
        printf("[ERR] %s: error loop\n", __FUNCTION__);
    }
}

void scrollingTimer(TimerHandle_t h)
{
    static bool scrol = true ;
    if(scrol)
        ssd1306_start_scroll_hori(&dev, false, 0, 7, FRAME_25);
    else
        ssd1306_stop_scroll(&dev);
    printf("[SYS] Scrolling status: %s\n", (scrol)? "On" : "Off");
    scrol=!scrol ;
}

void displayInit(void)
{
    printf("[SYS] Initiating SSD1603...\n");
    while (ssd1306_init(&dev) != 0)
    {
        printf("[ERR] %s: failed to init SSD1306 lcd\n", __func__);
        vTaskDelay(SECOND);
    }

    ssd1306_set_whole_display_lighting(&dev, true);
    vTaskDelay(SECOND);
    /* Create user interface task */
    xTaskCreate(displayTask, "displayTask", 256, NULL, 2, NULL);

    /* Scrolling timer */
    scrolTimeHandler = xTimerCreate("fps_timer", 10*SECOND, pdTRUE, NULL, scrollingTimer);
    xTimerStart(scrolTimeHandler, 0);

}
