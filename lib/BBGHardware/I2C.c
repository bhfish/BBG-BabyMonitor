#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>        // _Bool
#include <string.h>         // strerror
#include <errno.h>          // errno
#include <sys/ioctl.h>      // ioctl
#include <fcntl.h>          // open
#include <unistd.h>         // write
#include <linux/i2c-dev.h>  // I2C_SLAVE
#include <time.h>           // nanosleep
#include "I2C.h"

#define I2CDRV_FILE                         "/dev/i2c"
#define I2C_VIRTUAL_CAPE                    "BB-I2C"
#define I2C_VIRTUAL_CAPE_CONFIG_FILE_PATH   "/sys/devices/platform/bone_capemgr/slots"
#define I2C_SYS_FILE_SIZE                   50

// wait for I2C device specified by its bus number to be ready
static _Bool waitForI2CDeviceReady(int bus);

_Bool I2C_writeReg
(
    int I2CFd,              // [in] I2C write file descriptor
    unsigned char regAddr,  // [in] I2C device's register to write
    unsigned char memoryVal // [in] memory value to write
)
{
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = memoryVal;

    if (write(I2CFd, buff, 2) != 2) {
        printf("[ERROR] I2C: failed to register: %x with memory value: %x reason: %s\n", regAddr, memoryVal, strerror(errno));

        return false;
    }

    return true;
}

_Bool I2C_readSingleReg
(
    int I2CFd,                      // [in] I2C read file descriptor
    unsigned char regAddr,          // [in] I2C device's register to write
    char *memoryContent             // [in/out] pointer to hold register's memory content
)
{
    // To read a register, must first write the address
    if ( write(I2CFd, &regAddr, sizeof(regAddr)) != sizeof(regAddr) ) {
        printf("[ERROR] I2C: failed to write value: %p to register %x reason: %s\n", &regAddr, regAddr, strerror(errno));

        return false;
    }

    // Now read the value and return it
    if ( read(I2CFd, memoryContent, sizeof(memoryContent)) != sizeof(memoryContent) ) {
        printf("[ERROR] I2C: failed read register: %x's memory content reason: %s\n", regAddr, strerror(errno));

        return false;
    }

    return true;
}

_Bool I2C_readSequentialReg
(
    int I2CFd,                      // [in] I2C read file descriptor
    unsigned char startRegAddr,     // [in] starting device's register to read from
    int numOfReg,                   // [in] number of register to read in a sequence
    char *memoryContent             // [in/out] pinter to hold sequential registers' memory content
)
{
    if ( write(I2CFd, &startRegAddr, sizeof(startRegAddr)) != sizeof(startRegAddr) ) {
        printf("[ERROR] I2C: failed to write value: %p to register %x reason: %s\n", &startRegAddr, startRegAddr, strerror(errno));

        return false;
    }

    if (read(I2CFd, memoryContent, numOfReg) != numOfReg) {
        printf("[ERROR] I2C: failed read from register: %x reason: %s\n", startRegAddr, strerror(errno));

        return false;
    }

    return true;
}

int I2C_init
(
    int bus,            // [in] bus number of I2C device
    int deviceAddr      // [in] device I2C address
)
{
    int I2CFd;
    FILE *fPtr;
    char I2CBusFile[I2C_SYS_FILE_SIZE] = {0};
    char deviceVirtualCape[I2C_SYS_FILE_SIZE] = {0};
    sprintf(deviceVirtualCape, "%s%d", I2C_VIRTUAL_CAPE, bus);
    sprintf(I2CBusFile, "%s-%d", I2CDRV_FILE, bus);

    // load I2C virtual cape
    fPtr = fopen(I2C_VIRTUAL_CAPE_CONFIG_FILE_PATH, "w");

    if (fPtr == NULL) {
        printf("[ERROR] I2C: fopen %s failed reason %s\n", I2C_VIRTUAL_CAPE_CONFIG_FILE_PATH, strerror(errno));

        return -1;
    }

    if (fprintf(fPtr, "%s", deviceVirtualCape) <= 0) {
        printf("[ERROR] I2C: unable to load the I2C virtual cape %s\n", deviceVirtualCape);
        fclose(fPtr);

        return -1;
    }

    fclose(fPtr);

    if ( !waitForI2CDeviceReady(bus) ) {
        return -1;
    }


    I2CFd = open(I2CBusFile, O_RDWR);

    if (I2CFd < 0) {
        printf("[ERROR] I2C: open %s failed reason: %s\n", I2CBusFile, strerror(errno));

        return -1;
    }

    if (ioctl(I2CFd, I2C_SLAVE, deviceAddr) < 0) {
        printf("[ERROR] I2C: unable to set I2C device to slave address %d reason: %s\n", deviceAddr, strerror(errno));

        return -1;
    }

    return I2CFd;
}

static _Bool waitForI2CDeviceReady
(
    int bus  // [in] bus number of I2C device'
)
{
    struct timespec waitTime;
    const int MAX_WAIT_TIME = 5;
    int timeElapsed = 0;
    char I2CBusFile[I2C_SYS_FILE_SIZE] = {0};
    sprintf(I2CBusFile, "%s-%d", I2CDRV_FILE, bus);

    waitTime.tv_sec = 1;
    waitTime.tv_nsec = 0;

    while (open(I2CBusFile, O_RDWR) == -1) {
        if (timeElapsed == MAX_WAIT_TIME) {
            return false;
        }

        timeElapsed++;
        nanosleep(&waitTime, NULL);
    }

    return true;
}