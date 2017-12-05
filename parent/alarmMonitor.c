#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "joystick.h"
#include "buzzer.h"
#include "alarmMonitor.h"
#include "digitDisplay.h"
#include "tcpServer.h"
#include "util.h"
#include "watchDog.h"

//The maximum time buzzer sounds without push to stop
#define BUZZER_ALARM_TRIGGER_CNTR_MAX  300

//The maximum time buzzer sleeps without accepting alarm trigger signal
#define BUZZER_ALARM_SLEEP_CNTR_MAX  300

//Program state settings
static _Bool stopping = false;
static _Bool alarmTriggered = false;
static _Bool alarmSleep = false;
static _Bool alarmStateArm  = false;
static _Bool sysInitStatusFlag = false;
static int alarmBuzzMode   = 0;
static int babySoundLevel  = 0;
static int babyRoomTemp    = 0;
static dispMode_t currentDispMode = dispModeAlarmArm;
static pthread_t process_thread;


static void process(void);
static int processInit(void);
static void processCleanUp(void);


void AlarmMonitor_stopProg(void)
{
    stopping = true;
}

_Bool AlarmMonitor_isStopping(void)
{
    return stopping;
}

dispMode_t AlarmMonitor_getDispMode(void)
{
    return currentDispMode;
}

void AlarmMonitor_setDispMode(dispMode_t mode)
{
    currentDispMode = mode;
}

void AlarmMonitor_setAlarmBuzzMode(int mode)
{
    alarmBuzzMode = mode;
}

void AlarmMonitor_setAlarmTrigger(_Bool trigger)
{
    alarmTriggered = trigger;
}

_Bool AlarmMonitor_getAlarmTrigger(void)
{
    return alarmTriggered;
}

void AlarmMonitor_setAlarmArm(_Bool alarmArm)
{
    alarmStateArm = alarmArm;
}

_Bool AlarmMonitor_getAlarmArm(void)
{
    return alarmStateArm;
}

int AlarmMonitor_getAlarmBuzzMode(void)
{
    return alarmBuzzMode;
}

void AlarmMonitor_setBbyRoomTemp(int temp)
{
    babyRoomTemp = temp;
}

int AlarmMonitor_getBbyRoomTemp(void)
{
    return babyRoomTemp;
}

void AlarmMonitor_setBbySoundLevel(int sound)
{
    babySoundLevel = sound;
}

int AlarmMonitor_getBbySoundLevel(void)
{
    return babySoundLevel;
}

_Bool AlarmMonitor_getAlarmSleepStatus(void)
{
    return alarmSleep;
}

_Bool AlarmMonitor_getSysInitStatus(void)
{
    return sysInitStatusFlag;
}

static int processInit(void)
{
    int res = 0;

    //Initialize program state variables
    stopping = false;
    alarmTriggered = false;
    alarmSleep = false;
    alarmStateArm  = false;
    sysInitStatusFlag = false;
    alarmBuzzMode   = 0;
    babySoundLevel  = 0;
    babyRoomTemp    = 0;
    currentDispMode = dispModeAlarmArm;

    res = pthread_create(&process_thread, NULL,  (void *)&process, NULL);
    if( res ){
	printf("Thread process creation failed: %d\n", res);
	return -1;
    }

    return res;
}

static void processCleanUp(void)
{
    pthread_join( process_thread, NULL);
}

static void process(void)
{
    joystkDrctn_t jsDirPrev = JOYST_NONE;
    joystkDrctn_t jsDirCurrent = JOYST_NONE;
    static unsigned int alarmCntr = 0;

    while(!AlarmMonitor_isStopping()){

        jsDirCurrent = Joystick_getPressedDirection();

        if (jsDirPrev != jsDirCurrent){
            jsDirPrev = jsDirCurrent;

            if (JOYST_UP == jsDirCurrent){
                printf("joystick pushed up.\n");


                //Display next mode
                if(currentDispMode == dispModeSound)
                    currentDispMode = dispModeAlarmArm;
                else
                    currentDispMode++;

                printf("...[JS]currentDispMode: %d.\n", currentDispMode);

            }else if(JOYST_DOWN == jsDirCurrent){
                printf("joystick pushed down.\n");

                //Display previous mode
                if(currentDispMode == dispModeAlarmArm)
                    currentDispMode = dispModeSound;
                else
                    currentDispMode--;

                printf("...[JS]currentDispMode: %d.\n", currentDispMode);
            } else if(JOYST_PUSH == jsDirCurrent){
                printf("joystick pushed center.\n");

                if(alarmTriggered){
                    //Turn off the buzzer and disable the alarm
                    alarmTriggered = false;
                    alarmStateArm  = false;

                    printf("...[JS]alarmStateArm turned off.\n");
                }else if(currentDispMode == dispModeAlarmArm){
                    //Push joystick to arm/unarm the alarm
                    if(alarmStateArm  == false)
                        alarmStateArm = true;
                    else
                        alarmStateArm = false;

                    printf("...[JS]alarmStateArm: %d.\n", alarmStateArm);
                }
            }else if(JOYST_RIGHT == jsDirCurrent){
                printf("joystick pushed right.\n");

                if(currentDispMode == dispModeAlarmSound){
                    Buzzer_slectNextMode();
                }
            }else if(JOYST_LEFT == jsDirCurrent){
                printf("joystick pushed light.\n");

                if(currentDispMode == dispModeAlarmSound){
                    Buzzer_slectPrevMode();
                }
            }
        }

        if (alarmTriggered) {
            alarmCntr++;
            if (alarmCntr >= BUZZER_ALARM_TRIGGER_CNTR_MAX) {
                //Turn off buzzer ring
                alarmTriggered = false;

                //Enable alarm sleep mode
                alarmSleep = true;

                //reset counter
                alarmCntr = 0;
            }
        }else if (alarmSleep) {
            alarmCntr++;
            alarmTriggered = false;

            if (alarmCntr >= BUZZER_ALARM_SLEEP_CNTR_MAX) {
                //End sleep mode
                alarmSleep = false;

                //reset counter
                alarmCntr = 0;
            }
        } else {
            alarmCntr = 0;
        }

        //Delay
        sleep_msec(100);
    }
}


int main(int argc, char *argv[])
{
    int rt = 0;
    int watchDogRefID, watchDogTimer;
    _Bool wasRegistrationSuccess;
    
    if (rt == 0){
        rt = Buzzer_init();
    } else {
        printf("...[SysInit]Buzzer thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = DigitDisplay_init();
    } else {
        printf("...[SysInit]4-digit diplay thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = TCPServer_init();
    } else {
        printf("...[SysInit]TCP server thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = Joystick_init();
    } else {
        printf("...[SysInit]Joy stick thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = processInit();
    } else {
        printf("...[SysInit]main process thread creation failed: %d\n", rt);
    }

    //System start correctly
    if (rt == 0){
        printf("...[SysInit]System STARTED\n");
        sysInitStatusFlag = true;
    } else {
        return -1;
    }

    wasRegistrationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);

    if (!wasRegistrationSuccess) {
        printf("[ERROR] alarm thread unable to register to watch dog\n");
    } else {
        watchDogTimer = WatchDog_getWatchDogTimer();
    }

    while(!AlarmMonitor_isStopping()){
        //Delay
        sleep_sec(watchDogTimer - 5);

        if (wasRegistrationSuccess) {
            WatchDog_kickWatchDog(watchDogRefID);
        }
    }

    processCleanUp();
    TCPServer_cleanUp();
    Buzzer_cleanUp();
    DigitDisplay_cleanUp();

    return 0;
}