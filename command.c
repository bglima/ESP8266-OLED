#include "command.h"

/*
 * Initialize keys and command queues, both used by command tasks.
 */
void cmdInit() {
    keyQueue = xQueueCreate(MAX_LINE_SIZE, sizeof(char));
    cmdQueue = xQueueCreate(5, sizeof(char)*MAX_LINE_SIZE);
    printf("[SYS] keyQueue and cmdQueue are ready to go!\n");
    printf("[SYS] Let's test a hardcoded 'firstCommand' on the cmdQueue...\n");
    char testCmd [81] = "firstCommand";
    cmdRun( testCmd );
}

/*
 * Run a command based on a string
 */
void cmdRun( char *cmd ) {
    xQueueSend(cmdQueue, cmd, 300 / portTICK_PERIOD_MS);
}

/*
 * Task to handle the command line from cmdQueue.
 * Transform a line command into argc/argv pattern.
 */
void cmdHandlerTask(void *pvParameters)
{
    char cmd[MAX_LINE_SIZE];   // Max string size is 80. Last char must be \n or \r

    while(1) {
        /* Receive a command from cmdQueue */
        if( xQueueReceive(cmdQueue, &cmd[0], 1000 / portTICK_PERIOD_MS))
        {
            char *argv[MAX_ARGC];
            int argc = 1;
            char *temp, *rover;
            memset((void*) argv, 0, sizeof(argv));
            argv[0] = cmd;
            rover = cmd;

            /* Split string "<command> <argument 1> <argument 2>  ...  <argument N>"
             * into argv, argc style
             */
            while(argc < MAX_ARGC && (temp = strstr(rover, " "))) {
                rover = &(temp[1]);
                argv[argc++] = rover;
                *temp = 0;
            }

            /* Check if command is not empty */
            if (strlen(argv[0]) == 0)
                continue;

            /* Handle the execution. Firstly search for correct command... */
            printf("[SYS] Command arrived: %s\n", argv[0]);
        }

    }
}

/*
 * Read characters from keyQueue until a \n or \r.
 * Then, puts the whole line into cmdQueue.
 */
void cmdReaderTask(void *pvParameters)
{
    char rx;
    int i = 0;
    char cmd[MAX_LINE_SIZE];   // Max string size is 80. Last char must be \n or \r
    while(1)
    {
        // Receive a key from keyQueue
        if( xQueueReceive(keyQueue, &rx, 1000 / portTICK_PERIOD_MS) )
        {
            printf("%c", rx);
            fflush(stdout); // stdout is line buffered
            if (rx == '\n' || rx == '\r') {
                cmd[i] = 0;
                i = 0;
                printf("\n");

                // Send a command to cmd queue
                xQueueSend(cmdQueue, cmd, 300 / portTICK_PERIOD_MS);

                printf("%% ");
                fflush(stdout);
            } else {
                if (i < sizeof(cmd)) cmd[i++] = rx;
            }
        }
    }
}

/*
 * Read directly from keyboard. Put read characters into keyQueue
 */
void keyReaderTask(void *pvParameters)
{
    char ch;
    printf("\n\n\n[SYS] Running keyManagerTask. Type 'help<enter>' for, well, help\n");
    printf("%% ");
    fflush(stdout); // stdout is line buffered
    while(1) {
        if ( read(0, (void*)&ch, 1) ) {
            xQueueSend(keyQueue, &ch, 300 / portTICK_PERIOD_MS);
        }
    }
}
