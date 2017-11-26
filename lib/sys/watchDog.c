#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>            // _Bool
#include <pthread.h>            // pthread_*
#include <linux/watchdog.h>     // WDIOC_KEEPALIVE
#include <sys/ioctl.h>          // ioctl
#include <fcntl.h>              // open
#include <errno.h>              // errno
#include <string.h>             // strerror
#include "babyMonitor.h"

#define CONFIG_WATCH_DOG_TIME_OUT_IN_S          60
#define WATCH_DOG_KICK_INTERVAL_IN_S            40

// this module only expects to manage no more than 10 clients who tried to register to the watch dog service
#define MAX_NUM_REGISTERED_CLIENTS              10
#define LINUX_WATCH_DOG_PATH                    "/dev/watchdog"

typedef struct {
    _Bool isSlotVacant;
    _Bool wasKicked;
    int clientRefID;
}watchDogClients_t;

static watchDogClients_t watchDogCients[MAX_NUM_REGISTERED_CLIENTS];
static int currNumClients = 0;
static int watchDogFD;
static pthread_t watchDogThread;
static pthread_mutex_t watchDogMutex = PTHREAD_MUTEX_INITIALIZER;

static int getRandomNum(int x);
static void kickWatchDog(void);
static void setWatchDogTimeout(const int NEW_TIMEOUT);
static void *startWatchDogThread(void *args);

_Bool WatchDog_initWatchDog(void)
{
    srand(time(NULL));

    for (int i = 0; i < MAX_NUM_REGISTERED_CLIENTS; ++i)
    {
        watchDogCients[i].isSlotVacant = true;
        watchDogCients[i].wasKicked = false;
        watchDogCients[i].clientRefID = getRandomNum(i);
    }

    watchDogFD = open(LINUX_WATCH_DOG_PATH, O_RDWR);

    if (watchDogFD == -1) {
        printf("[ERROR] failed to open %s reason: %s\n", LINUX_WATCH_DOG_PATH, strerror(errno));

        return false;
    }

    setWatchDogTimeout(CONFIG_WATCH_DOG_TIME_OUT_IN_S);

    if (pthread_create(&watchDogThread, NULL, startWatchDogThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in watchDog module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

_Bool WatchDog_registerToWatchDog(int *clientRefIDPtr)
{
    _Bool wasRegisterationSuccess;

    if (currNumClients > MAX_NUM_REGISTERED_CLIENTS) {
        printf("[ERROR] unable to register to watch dog service\n");

        return false;
    }

    for (int i = 0; i < MAX_NUM_REGISTERED_CLIENTS; ++i)
    {
        pthread_mutex_lock(&watchDogMutex);
        {
            if (watchDogCients[i].isSlotVacant) {
                watchDogCients[i].isSlotVacant = false;
                *clientRefIDPtr = watchDogCients[i].clientRefID;
                wasRegisterationSuccess = true;
                currNumClients++;
            }
        }
        pthread_mutex_unlock(&watchDogMutex);

        if (wasRegisterationSuccess) {
            return wasRegisterationSuccess;
        }
    }

    wasRegisterationSuccess = false;

    printf("[ERROR] unable to register to watch dog service\n");

    return wasRegisterationSuccess;
}

void WatchDog_deregisterFromWatchDog(int clientRefID)
{
    _Bool wasDeregisterationSuccess = false;

    for (int i = 0; i < MAX_NUM_REGISTERED_CLIENTS; ++i)
    {
        pthread_mutex_lock(&watchDogMutex);
        {
            if (watchDogCients[i].clientRefID == clientRefID) {
                watchDogCients[i].isSlotVacant = true;
                watchDogCients[i].clientRefID = getRandomNum(i);
                wasDeregisterationSuccess = true;
                currNumClients--;
            }
        }
        pthread_mutex_unlock(&watchDogMutex);

        if (wasDeregisterationSuccess) {
            return;
        }
    }

    printf("[WARN] watchDog: client reference ID %d unrecognized\n", clientRefID);
}

void WatchDog_kickWatchDog(int clientRefID)
{
    _Bool wasKickSuccessful = false;

    for (int i = 0; i < MAX_NUM_REGISTERED_CLIENTS; ++i)
    {
        pthread_mutex_lock(&watchDogMutex);
        {
            if (watchDogCients[i].clientRefID == clientRefID) {
                watchDogCients[i].wasKicked = true;
                wasKickSuccessful = true;
                printf("client reference ID %d requested to kick the watchdog\n", clientRefID);
            }
        }
        pthread_mutex_unlock(&watchDogMutex);

        if (wasKickSuccessful) {
            return;
        }
    }

    printf("[WARN] watchDog: client reference %d unrecognized\n", clientRefID);
}

int WatchDog_getWatchDogTimer(void)
{
    /*
        this value is hard coded because it has to be less than the timeout which configured by this module (60s) in order
        to make sure other module CAN kick the watch dog in time
    */
    return 20;
}

static void setWatchDogTimeout(const int NEW_TIMEOUT)
{
    ioctl(watchDogFD, WDIOC_SETTIMEOUT, &NEW_TIMEOUT);
}

static void kickWatchDog(void)
{
    ioctl(watchDogFD, WDIOC_KEEPALIVE, NULL);

    printf("just kicked the watch dog\n");
}

// simple random generator
static int getRandomNum(int x)
{
    int randomNum = rand() + x / (37 + x);

    return randomNum;
}

// define the duty of watch dog thread
static void *startWatchDogThread(void *args)
{
    _Bool isOKToKick = false;
    struct timespec kickTime;
    struct timespec remainTime;

    kickTime.tv_sec = WATCH_DOG_KICK_INTERVAL_IN_S;
    kickTime.tv_nsec = 0;

    while (true) {
        nanosleep(&kickTime, &remainTime);

        pthread_mutex_lock(&watchDogMutex);
        {
            for (int i = 0; i < MAX_NUM_REGISTERED_CLIENTS; ++i)
            {
                if (!watchDogCients[i].isSlotVacant) {
                    isOKToKick = watchDogCients[i].wasKicked;
                    watchDogCients[i].wasKicked = false;
                }
            }
        }
        pthread_mutex_unlock(&watchDogMutex);

        if (isOKToKick) {
            kickWatchDog();
        }

        // isOKToKick is an indication/flag to tell whether all registered clients to the watch dog service are running or not
        BabayMonitor_setSystemRunningStatus(isOKToKick);
    }

    // bad things happened...
    pthread_exit(PTHREAD_CANCELED);
}
