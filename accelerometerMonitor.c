#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // close
#include <time.h>       // nanosleep
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <stdint.h>     // int16_t
#include "sender.h"
#include "I2C.h"

#define ACCELEROMETER_BUS_NUM               1
#define ACCELEROMETER_I2C_ADDR              0x1c
#define ACCELEROMETER_CTRL_REG1_ADDR        0x2a    // register for turn on/off active mode of data rate
#define ACCELEROMETER_STATUS_REG_ADDR       0x00    // register for checking which axises was just overwritten
#define NUM_OF_DATA_REGISTERS               7
#define MONITOR_TIME_INTERVAL_IN_S          1

static pthread_t accelerometerThread;
static _Bool stopMonitoring = false;
static int I2CFD;

static void *startAccelerometerThread(void *args);

/*
    NOTE: Reading the MSB register and then the LSB register in sequence ensures that both bytes (LSB and MSB)
          belong to the same data sample, even if a new data sample arrives between reading the MSB and the LSB byte
*/
enum ACCELEROMETER_XYZ_DATA{
    STATUS,
    OUT_X_MSB,
    OUT_X_LSB,
    OUT_Y_MSB,
    OUT_Y_LSB,
    OUT_Z_MSB,
    OUT_Z_LSB
};

_Bool AccelerometerMonitor_startMonitoring(void)
{
    I2CFD = I2C_init(ACCELEROMETER_BUS_NUM, ACCELEROMETER_I2C_ADDR);

    if (I2CFD < 0) {
        printf("I2C_init failed\n");

        return false;
    }

    // set data rate register to be standby
    if ( !I2C_writeReg(I2CFD, ACCELEROMETER_CTRL_REG1_ADDR, 0x00) ) {
        printf("failed to set data rate register to be standby\n");
        close(I2CFD);

        return false;
    }

    // set data rate register to be active
    if ( !I2C_writeReg(I2CFD, ACCELEROMETER_CTRL_REG1_ADDR, 0x01) ) {
        printf("failed to set data rate register to be active\n");
        close(I2CFD);

        return false;
    }

    if (pthread_create(&accelerometerThread, NULL, &startAccelerometerThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in accelerometerMonitor module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

void AccelerometerMonitor_stopMonitoring(void)
{
    void *accelerometerThreadExitStatus;

    stopMonitoring = true;
    close(I2CFD);

    if (pthread_join(accelerometerThread, &accelerometerThreadExitStatus) != 0) {
        printf("[ERROR] failed to join with terminated accelerometer thread failed reason: %s\n", strerror(errno));
    }

    if (accelerometerThreadExitStatus != PTHREAD_CANCELED)
    {
        // bad things happened...
        printf("[ERROR] abnormal termination state of accelerometer thread\n");
        printf("try to send a cancel request and wish accelerometer thread can normally be terminated...\n");

        if(pthread_cancel(accelerometerThread) != 0)
        {
            printf("[ERROR] failed to send a cancel request to accelerometer thread failed reason: %s\n", strerror(errno));
        }
    }
}

// define the duty of accelerometer thread
static void *startAccelerometerThread
(
    void *args  // [in] list of argument passed by pthread_create()
)
{
    char xyzDataRegContent[NUM_OF_DATA_REGISTERS] = {0};
    // int16_t xVal, yVal, zVal;
    struct timespec monitorTime;
    struct timespec remainTime;

    monitorTime.tv_sec = MONITOR_TIME_INTERVAL_IN_S;
    monitorTime.tv_nsec = 0;

    while(!stopMonitoring)
    {
        // read from status register to out_z_lsb which contains a total of 7 register
        if ( !I2C_readSequentialReg(I2CFD, ACCELEROMETER_STATUS_REG_ADDR, NUM_OF_DATA_REGISTERS, xyzDataRegContent) ) {
            printf("failed to read x,y,z axises status\n");

            break;
        }

        // xVal = (xyzDataRegContent[OUT_X_MSB] << 8) | (xyzDataRegContent[OUT_X_LSB]);
        // yVal = (xyzDataRegContent[OUT_Y_MSB] << 8) | (xyzDataRegContent[OUT_Y_LSB]);
        // zVal = (xyzDataRegContent[OUT_Z_MSB] << 8) | (xyzDataRegContent[OUT_Z_LSB]);

        // TODO detect accelerations and raise an alarm to parent's BBG
        // printf("--------------------\n");
        // printf("x value: %d\n", xVal);
        // printf("y value: %d\n", yVal);
        // printf("z value: %d\n", zVal);
        // printf("--------------------\n");
        // printf("\n");
        // printf("\n");

        Sender_sendDataToParentBBG(-1, ACCELERATION, true);
        nanosleep(&monitorTime, &remainTime);
    }

    pthread_exit(PTHREAD_CANCELED);
}

