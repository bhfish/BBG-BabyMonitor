#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    //_Bool
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <dirent.h>     // DIR, opendir

#define GPIO_FILE_SYSTEM    "/sys/class/gpio"
#define GPIO_SYS_FILE_LENGTH  50

_Bool GPIO_initPin(int GPIONum)
{
    char GPIOPinDir[GPIO_SYS_FILE_LENGTH] = {0};
    char GPIOExportFile[GPIO_SYS_FILE_LENGTH] = {0};
    sprintf(GPIOPinDir, "%s/gpio%d", GPIO_FILE_SYSTEM, GPIONum);
    sprintf(GPIOExportFile, "%s/export", GPIO_FILE_SYSTEM);

    DIR *dPtr = opendir(GPIOPinDir);

    if (dPtr != NULL) {
        closedir(dPtr);

        return true;
    }

    // ENOENT: Directory does not exist, or name is an empty string.
    if (errno == ENOENT) {
        // Use fopen() to open the file for write access.
        FILE *fPtr = fopen(GPIOExportFile, "w");

        if (fPtr == NULL) {
            printf("ERROR: GPIO fopen %s failed reason: %s\n", GPIOExportFile, strerror(errno));

            return false;
        }

        // Write to data to the file using fprintf():
        if (fprintf(fPtr, "%d", GPIONum) <= 0) {
            printf("ERROR: GPIO unable to export pin: %d as GPIO\n", GPIONum);
            fclose(fPtr);

            return false;
        }

        fclose(fPtr);

        return true;
    }
    else {
        printf("ERROR: GPIO opendir %s failed reason: %s\n", GPIOPinDir, strerror(errno));

        return false;
    }
}

_Bool GPIO_setPinAsInput(int GPIONum)
{
    char GPIODirectFile[GPIO_SYS_FILE_LENGTH] = {0};
    sprintf(GPIODirectFile, "%s/gpio%d/direction", GPIO_FILE_SYSTEM, GPIONum);

    FILE *fPtr = fopen(GPIODirectFile, "w");

    if (fPtr == NULL) {
        printf("ERROR: GPIO fopen %s failed reason: %s\n", GPIODirectFile, strerror(errno));

        return false;
    }

    if (fprintf(fPtr, "in") <= 0) {
        printf("ERROR: GPIO unable to set the GPIO pin: %d as input\n", GPIONum);
        fclose(fPtr);

        return false;
    }

    return true;
}

_Bool GPIO_getInputPinStatus(int GPIONum, int *pinStatus)
{
    char joystickStatus[GPIO_SYS_FILE_LENGTH] = {0};
    char joystickStatusFile[GPIO_SYS_FILE_LENGTH] = {0};
    sprintf(joystickStatusFile, "%s/gpio%d/value", GPIO_FILE_SYSTEM, GPIONum);

    FILE *fPtr = fopen(joystickStatusFile, "r");

    if (fPtr == NULL) {
        printf("ERROR: GPIO fopen %s failed reason: %s\n", joystickStatusFile, strerror(errno));

        return false;
    }

    // the reason not to read only one char from the value file is there is a chance that the value file is contaminated: more than just 0 or 1 value
    fgets(joystickStatus, GPIO_SYS_FILE_LENGTH, fPtr);

    // replace the end line char to null terminate char
    joystickStatus[strlen(joystickStatus) - 1] = '\0';

    /*
        from hardware point of view, 0 = pressed, 1 = released and again, in case the value file was contaminated,
        compare the whole line string instead of the only first character
    */
    if (strcmp(joystickStatus, "0") == 0 || strcmp(joystickStatus, "1") == 0) {
        *pinStatus = atoi(joystickStatus);
    }
    else {
        // the value file may be contaminated for some unexpected reason
        printf("ERROR: GPIO Unable to read the joystick status\n");
        fclose(fPtr);

        return false;
    }

    fclose(fPtr);

    return true;
}