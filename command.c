#include "command.h"

/*
 * Show all avaliable commands within invoker
 */
static status_t cmdHelp(uint32_t argc, char *argv[]) {
    printf("[SYS] List of avaliable commands: \n");
        for(int i = 0; i < currentNumOfCommands; ++i)
            printf("%s", invoker[i].cmdHelp);
        printf("\n\n");
    return OK;
}

/*
 * Sleep for 2 seconds without interfeering in other tasks
 */
status_t cmdSleep(uint32_t argc, char *argv[])
{
    printf("[SYS] I'll sleep 2 seconds. Let's test UART HW FIFO\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    return OK;
}

/*
 * Initialize keys and command queues, both used by command tasks.
 */
void cmdInit() {
    keyQueue = xQueueCreate(MAX_LINE_SIZE, sizeof(char));
    cmdQueue = xQueueCreate(5, sizeof(char)*MAX_LINE_SIZE);

    commandDescriptor_t descriptorHelp = {"help", &cmdHelp, " $help     Show all avaliable commands\n"};
    commandDescriptor_t descriptorSleep = {"sleep", &cmdSleep, " $sleep     Take a nap for two seconds\n"};
    cmdInsert(descriptorHelp);
    cmdInsert(descriptorSleep);
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
            int cmdIndex;
            status_t cmdStatus = FAIL;
            for( cmdIndex = 0; cmdIndex < currentNumOfCommands; ++cmdIndex )
                if ( strcmp(argv[0], invoker[cmdIndex].cmdString) == 0 ) break;

            /* Check if command was found */
            if ( cmdIndex == currentNumOfCommands )
                printf("[ERR] Unknown command '%s'! Try running 'help'\n", argv[0]);
            else {
                printf("[SYS] Command found! Running now...\n");
                cmdStatus = (*invoker[cmdIndex].receiver)(argc, argv);
            }

            /* Check command result */
            if( cmdStatus == OK )
                printf("[SYS] Command successful\n");
            else if ( cmdStatus == FAIL )
                printf("[SYS] Command failed\n");
            else
                printf("[SYS] Unknown command state\n");
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

/*
 * Insert a new command in command descriptor
 */
bool cmdInsert(commandDescriptor_t newCmd)
{
    if (currentNumOfCommands >= MAX_NUM_OF_CMD)
        return false;

    invoker[currentNumOfCommands++] = newCmd;
    return true;
}
