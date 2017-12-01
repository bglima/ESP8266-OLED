#ifndef MPU5060_HEADER
#define MPU5060_HEADER

#include "stdint.h"
#include "stdio.h"
#include "i2c.h"
#include "math.h"
#include "command.h"
#include "task.h"

/* General definitions */
#define ADDR 0x68
#define WHO_I_AM     0x75 // decimal 117
#define PWR_MGMT_1   0x6B // decimal 107. SLEEP
#define GYRO_CONFIG  0x1B // decimal 27
#define ACCEL_CONFIG 0x1C // decimal 28
#define ACCEL_XOUT_H 0x3B // decimal 59
#define FIFO_R_W 0x74     // decimal 116
#define FIFO_COUNTH   0x72
#define GRAVITY 9.826
#define ACCELEROMETER_SENSITIVITY 16384.0
#define GYROSCOPE_SENSITIVITY 131.0
#define TEMPERATURE_SENSIVITY 340.0
#define MAX_PACKET_SIZE 10
typedef union
{
    uint8_t buffer[14];         // Accessing the whole buffer

    struct                      // Accessing each value individually
    {
        int16_t accelX : 16;
        int16_t accelY : 16;
        int16_t accelZ : 16;
        int16_t temp : 16;
        int16_t gyroX : 16;
        int16_t gyroY : 16;
        int16_t gyroZ : 16;
    } value;

    struct                      // Acessing them as vectors
    {
        int16_t accel[3];
        int16_t temp;
        int16_t gyro[3];
    } vec;

} mpuData_t;

/* Attributes */
static mpuData_t mpuRawData;
static float accel[3];
static float gyro[3];
static float lastAccel[3];
static float lastGyro[3];
static float temp;
static float dt;
static int printFreq;

/* Basic read functions */
void mpuInit();             /* Wake MPU and setup GYRO and ACCEL */
uint8_t mpuCheck();         /* Reads WHO_I_AM register from MPU and checks whether it is in SLEEP mode */
bool mpuReadValues();       /* Reads dev data and fill mpu_data buffer. Returns success of operation */
bool mpuTapped(float thresh);/* Returns whether the accel was tilter or not */
float mpuGetTemp();         /* Return the temperature from last reading */
void printData();           /* Print converted data on screen */

/* Timming functions */
bool mpuSetDt(float new_dt);  /* Min value 0.0001 and max value 1. Default is 0.01. */
float mpuGetDt();             /* Return dt */

/*
 * User related functions
 */
status_t cmdCheckMpu(uint32_t argc, char *argv[]);
status_t cmdPrintMpu(uint32_t argc, char *argv[]);
status_t cmdStreamMpu(uint32_t argc, char *argv[]);
status_t cmdStreamCloseMpu(uint32_t argc, char *argv[]);

/*
 * Queues and Tasks
 */
static QueueHandle_t packetQueue;
void getPacketTask(void *pvParameters);
void handlePacketTask(void *pvParameters);
void printDataTask(void *pvParameters);

#endif
