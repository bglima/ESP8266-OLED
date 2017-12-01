#include "MPU6050.h"

/*
 * Init MPU5060 and configure ACCELL and GYRO data.
 * Return true if sucessfuly configured
 */
void mpuInit() {
    int regs[] = {PWR_MGMT_1, GYRO_CONFIG, ACCEL_CONFIG};
    for (int i=0;i<3;i++) {
        uint8_t data[] = {regs[i], 0};  /* Set each config reg to zero */
        bool success = i2c_slave_write(ADDR, data, sizeof(data));
        if (!success)
            return;     /* If any of registers does not respond, cancel request */
    }

    mpuReadValues();

    /* Init last values as current values at first */
    for(int i=0; i < 3; ++i) {
        lastAccel[i] = accel[i];
        lastGyro[i] = accel[i];
    }

    /* Set commands for MPU at invoker */
    commandDescriptor_t descriptorCheckMpu = {"check-mpu", &cmdCheckMpu, " $check-mpu     Print MPU status on the screen.\n"};
    commandDescriptor_t descriptorPrintMpu = {"print-mpu", &cmdPrintMpu, " $print-mpu     Print MPU last read data.\n"};
    cmdInsert(descriptorCheckMpu);
    cmdInsert(descriptorPrintMpu);
}



/*
 * Internal command functions
 */

/* Check MPU status. 0 = OK; 1 = SLEEPING; 2 = NOT_FOUND */
uint8_t mpuCheck()
{
    uint8_t reg_data = -1;

    /* Read WHO_I_AM register */
    i2c_slave_read(ADDR, WHO_I_AM, &reg_data, 1);
    if ( reg_data != 0x68 ) /* Wrong address or not found */
        return 2;

    /* Read PWR_MGMT_1 register */
    i2c_slave_read(ADDR, PWR_MGMT_1, &reg_data, 1);
    if ( reg_data == 0x64 ) /* MPU found, but in SLEEP mode */
        return 1;

    /* If it reaches here, MPU is active */
    return 0;
}

/* Read data from MPU and fill mpuRawData buffer */
bool mpuReadValues()
{
    uint8_t buffer[14];

    if( mpuCheck() ) // If return 0, MPU is active
              return false;

          bool ok = i2c_slave_read(ADDR, ACCEL_XOUT_H, buffer, 14);
          if( !ok )
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
    printf("\n[SYS] --> Printing MPU5060 read values: <--\n");
    printf("[SYS] Accel (m/s^2) X: %.2f; Y:%.2f; Z:%.2f\n", accel[0], accel[1], accel[2]);
    printf("[SYS] Gyro  (m/s^2) X: %.2f; Y:%.2f; Z:%.2f\n", gyro[0], gyro[1], gyro[2]);
    printf("[SYS] Temp  (Celsius): %f \n", temp);
    printf("\n");
    return OK;
}
