#include "MPU6050.h"

/*
 * Init MPU5060 and configure ACCELL and GYRO data.
 * Return true if sucessfuly configured
 */
void mpuInit(QueueHandle_t *newTapQueue) {
    uint8_t regs[3] = {PWR_MGMT_1, GYRO_CONFIG, ACCEL_CONFIG};
    for (int i=0;i<3;i++) {
        uint8_t data = 0;  /* Set each config reg to zero */
        int err = i2c_slave_write(I2C_BUS, MPU_ADDR, &(regs[i]), &data, sizeof(data));
        if ( err )
            return;     /* If any of registers does not respond, cancel request */
    }

    /* Set external queue pointer to tap */
    internalTapQueue = newTapQueue;
    printf("[SYS] MPU config was set successfuly!\n");

    /* Read once and fill initial values */
    mpuReadValues();
    printData();

    /* Init last values as current values at first */
    for(int i=0; i < 3; ++i) {
        lastAccel[i] = accel[i];
        lastGyro[i] = accel[i];
    }

    /* Timming related definitions */
    dt = 0.01;
    printFreq = 0;

    packetQueue =  xQueueCreate(MAX_PACKET_QUEUE_SIZE, sizeof(mpuData_t));
    xTaskCreate(handlePacketTask, "handlePacketTask", 512, NULL, 2, NULL);
    xTaskCreate(getPacketTask, "getPacketTask", 512, NULL, 3, NULL);
    xTaskCreate(printDataTask, "printDataTask", 512, NULL, 1, NULL);
    xTaskCreate(getTapTask, "getTapTask", 512, NULL, 2, NULL);

    /* Tap definitions */
    tapEnabled = true;
    tickSinceTap = 0;
    tapTreshold = 10.0;

    /* Set commands for MPU at invoker */
    commandDescriptor_t descriptorCheckMpu = {"mpu-check", &cmdCheckMpu, " $mpu-check     Print MPU status on the screen.\n"};
    commandDescriptor_t descriptorPrintMpu = {"mpu-print", &cmdPrintMpu, " $mpu-print     Print MPU last read data.\n"};
    commandDescriptor_t descriptorStreamMpu = {"mpu-stream", &cmdStreamMpu, " $mpu-stream <frequency>   Show <frequency> MPU data per second.\n"};
    commandDescriptor_t descriptorStreamCloseMpu = {"x", &cmdStreamCloseMpu, " $x   Close priting stream.\n"};
    commandDescriptor_t descriptorSetTapMpu = {"mpu-set-tap", &cmdSetTapMpu, " $mpu-set-tap   Set threshold for tapping.\n"};
    cmdInsert(descriptorCheckMpu);
    cmdInsert(descriptorPrintMpu);
    cmdInsert(descriptorStreamMpu);
    cmdInsert(descriptorStreamCloseMpu);
    cmdInsert(descriptorSetTapMpu);
}

/*
 *
 * Internal command functions
 *
 */

/* Check MPU status. 0 = OK; 1 = SLEEPING; 2 = NOT_FOUND */
uint8_t mpuCheck()
{
    uint8_t reg_data = -1;

    /* Read WHO_I_AM register */
    uint8_t regs[2] = {WHO_I_AM, PWR_MGMT_1};

    int err = i2c_slave_read(I2C_BUS, MPU_ADDR, &regs[0], &reg_data, 1);
    if ( reg_data != 0x68 ) /* Wrong address or not found */
        return 2;

    /* Read PWR_MGMT_1 register */
    err = i2c_slave_read(I2C_BUS, MPU_ADDR, &regs[1], &reg_data, 1);
    if ( reg_data == 0x64 ) /* MPU found, but in SLEEP mode */
        return 1;

    /* If it reaches here, MPU is active */
    return 0;
}

/* Read data from MPU and fill mpuRawData buffer
 *
 * ==> DEPRECATED! NOW, DATA IS READ FROM TASKS <==
 */
bool mpuReadValues()
{
    uint8_t buffer[14];

    if( mpuCheck() ) // If return 0, MPU is active
        return false;

    uint8_t reg = ACCEL_XOUT_H;

    int err = i2c_slave_read(I2C_BUS, MPU_ADDR, &reg, buffer, 14);
    if( err )
        return false;

    mpuRawData.value.accelX = (((int16_t)buffer[0]) << 8) | buffer[1];
    mpuRawData.value.accelY = (((int16_t)buffer[2]) << 8) | buffer[3];
    mpuRawData.value.accelZ = (((int16_t)buffer[4]) << 8) | buffer[5];
    mpuRawData.value.temp = (((int16_t)buffer[6]) << 8) | buffer[7];
    mpuRawData.value.gyroX = (((int16_t)buffer[8]) << 8) | buffer[9];
    mpuRawData.value.gyroY = (((int16_t)buffer[10]) << 8) | buffer[11];
    mpuRawData.value.gyroZ = (((int16_t)buffer[12]) << 8) | buffer[13];

    float accelFactor = GRAVITY / ACCELEROMETER_SENSITIVITY;
    float gyroFactor = 1 / GYROSCOPE_SENSITIVITY;
    float tempFactor = 1 / TEMPERATURE_SENSIVITY;
    accel[0] = mpuRawData.value.accelX * accelFactor;
    accel[1] = mpuRawData.value.accelY * accelFactor;
    accel[2] = mpuRawData.value.accelZ * accelFactor;
    temp = mpuRawData.value.temp * tempFactor + 36.53;
    gyro[0] = mpuRawData.value.gyroX * gyroFactor;
    gyro[1] = mpuRawData.value.gyroY * gyroFactor;
    gyro[2] = mpuRawData.value.gyroZ * gyroFactor;
    return true;
}

/* Return last read temperature in Celsius */
float mpuGetTemp()
{
    return temp;
}

/* Self-explained */
void printData()
{
    printf("\n[SYS] --> Printing MPU5060 read values: <--\n");
    printf("[SYS] Accel (m/s^2) X: %.2f; Y:%.2f; Z:%.2f\n", accel[0], accel[1], accel[2]);
    printf("[SYS] Gyro  (m/s^2) X: %.2f; Y:%.2f; Z:%.2f\n", gyro[0], gyro[1], gyro[2]);
    printf("[SYS] Temp  (Celsius): %f \n", temp);
    printf("\n");
}

/* Returns true if accel was tilted */
bool tapped(float thresh)
{
    float diffAccelZ = fabs(lastAccel[2] - accel[2]);

    if (  diffAccelZ > thresh )
        return true;
    return false;
}


/*
 * User command functions
 */
status_t cmdCheckMpu(uint32_t argc, char *argv[])
{
   uint8_t status = mpuCheck();
   switch (status) {
   case 2:
       printf("[SYS] MPU not found...\n");
       break;
   case 1:
       printf("[SYS] MPU is in sleep mode!\n");
       break;
   case 0:
       printf("[SYS] MPU is active!\n");
       break;
   }
   return OK;
}

status_t cmdPrintMpu(uint32_t argc, char *argv[])
{
    printData();
    return OK;
}

status_t cmdStreamMpu(uint32_t argc, char *argv[])
{
    if(argc < 2) {
        printf("[ERR] Missing printf frequency. Please choose between 0 and 5\n");
        return FAIL;
    }

    int newPrintFreq = atoi(argv[1]);
    if ( newPrintFreq < 0 || newPrintFreq > 5 ) {
        printf("[ERR] Please, choose a frequency between 0 and 5. \n");
        return FAIL;
    }
    printFreq = newPrintFreq;
    printf("[SYS] New print frequency set to %i\n", printFreq);
    return OK;
}

status_t cmdStreamCloseMpu(uint32_t argc, char *argv[])
{
    printFreq = 0;
    printf("[SYS] Closed print stream\n");
    return OK;
}

status_t cmdSetTapMpu(uint32_t argc, char *argv[])
{
    if(argc < 2) {
        printf("[ERR] Missing tap threshold. Please choose between 0.0 and 15.0\n");
        return FAIL;
    }

    float newTapThreshold = atof(argv[1]);
    if ( newTapThreshold < 0 || newTapThreshold > 15 ) {
        printf("[ERR] Please, choose a tap threshold between 0 and 15.0\n");
        return FAIL;
    }
    tapTreshold = atof(argv[1]);
    printf("[SYS] Tap threshold set to %.2f\n", tapTreshold);
    return OK;
}

/*
 *
 * Main tasks rountines
 *
 */
void getPacketTask(void *pvParameters)
{
    uint8_t buffer[14];
    uint8_t reg = ACCEL_XOUT_H;
    while(1) {
        if ( i2c_slave_read(I2C_BUS, MPU_ADDR, &reg, buffer, 14) == 0 ) {    /* 0 means SUCCESS */
             xQueueSend(packetQueue, buffer, 100 / portTICK_PERIOD_MS);
        }
        vTaskDelay( dt * 1000 / portTICK_PERIOD_MS );
    }

}

void handlePacketTask(void *pvParameters)
{
    uint8_t buffer[14];
    while(1) {
        if ( xQueueReceive( packetQueue, buffer, 100 / portTICK_PERIOD_MS) ) {

            mpuRawData.value.accelX = (((int16_t)buffer[0]) << 8) | buffer[1];
            mpuRawData.value.accelY = (((int16_t)buffer[2]) << 8) | buffer[3];
            mpuRawData.value.accelZ = (((int16_t)buffer[4]) << 8) | buffer[5];
            mpuRawData.value.temp = (((int16_t)buffer[6]) << 8) | buffer[7];
            mpuRawData.value.gyroX = (((int16_t)buffer[8]) << 8) | buffer[9];
            mpuRawData.value.gyroY = (((int16_t)buffer[10]) << 8) | buffer[11];
            mpuRawData.value.gyroZ = (((int16_t)buffer[12]) << 8) | buffer[13];

            float accelFactor = GRAVITY / ACCELEROMETER_SENSITIVITY;
            float gyroFactor = 1 / GYROSCOPE_SENSITIVITY;
            float tempFactor = 1 / TEMPERATURE_SENSIVITY;
            accel[0] = mpuRawData.value.accelX * accelFactor;
            accel[1] = mpuRawData.value.accelY * accelFactor;
            accel[2] = mpuRawData.value.accelZ * accelFactor;
            temp = mpuRawData.value.temp * tempFactor + 36.53;
            gyro[0] = mpuRawData.value.gyroX * gyroFactor;
            gyro[1] = mpuRawData.value.gyroY * gyroFactor;
            gyro[2] = mpuRawData.value.gyroZ * gyroFactor;

        }
    }
}

void printDataTask(void *pvParameters)
{
    while(1)
    {
        while ( printFreq == 0 )
            taskYIELD();

        printData();
        vTaskDelay( (1000 / printFreq) / portTICK_PERIOD_MS );
    }
}

void getTapTask(void *pvParameters)
{
    while( 1 )
    {
        while( !tapEnabled )
            taskYIELD();

        vTaskDelay( (dt * 1000) / portTICK_PERIOD_MS ) ;
        int tapCode;

        /* Check and update tickSinceTap */
        if ( tickSinceTap > 0 )
            --tickSinceTap;

        if (tickSinceTap == 1) {        /* Finished waiting for second tap */
            printf("[SYS] Was tapped!\n");
            tapCode = 0; /* Zero (0) means one single tap */
            xQueueSend(*internalTapQueue, &tapCode, 0 / portTICK_PERIOD_MS);
            tickSinceTap = 0;
            continue;
        }

        /* Check if  tapped! */
        if ( tapped( tapTreshold ) ) {

           /* If not waiting for second tap yet... */
           if ( tickSinceTap == 0 )
               tickSinceTap = MAX_TAP_LENGTH;   /* Start waiting... */
           else {
              printf("[SYS] Was double-tapped!\n");
              tapCode = 1; /* One (1) means a double-tap */
              xQueueSend(*internalTapQueue, &tapCode, 0 / portTICK_PERIOD_MS);
              tickSinceTap = 0;
           }
           vTaskDelay( 150 / portTICK_PERIOD_MS ) ;
        }
    }
}





