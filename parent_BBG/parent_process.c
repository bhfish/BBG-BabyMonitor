#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "parent_joystick.h"
#include "parent_buzzer.h"
#include "parent_process.h"
#include "4digi_display.h"
#include "tcp_server.h"
#include "seg_disp.h"

//Threshold to trigger alarm
//#define SOUND_ALARM_TRIGGER_THRESHOLD  80

//The maximum time buzzer sounds without push to stop
#define BUZZER_ALARM_TRIGGER_CNTR_MAX  300

//The maximum time buzzer sleeps without accepting alarm trigger signal
#define BUZZER_ALARM_SLEEP_CNTR_MAX  300

//Program state settings
bool stopping = false;
bool alarmTriggered = false;
static bool alarmSleep = false;
bool alarmStateArm  = false;
bool sysInitStatusFlag = false;
int alarmBuzzMode   = 0;
int babySoundLevel  = 0;
int babyRoomTemp    = 0;
dispMode_t currentDispMode = dispModeAlarmArm;


dispMode_t getDispMode(void)
{
    return currentDispMode;
}

void setBbySoundLevel(int sound)
{
    babySoundLevel = sound;
}

int getBbySoundLevel(void)
{
    return babySoundLevel;
}

bool getAlarmSleepStatus(void)
{
    return alarmSleep;
}

bool getSysInitStatus(void)
{
    return sysInitStatusFlag;
}

void process(void)
{
//	static int i = 1;

    joystkDrctn_t jsDirPrev = JOYST_NONE;
    joystkDrctn_t jsDirCurrent = JOYST_NONE;
    static unsigned int alarmCntr = 0;

	while(!stopping)
	{
//        if (babySoundLevel > SOUND_ALARM_TRIGGER_THRESHOLD){
//            alarmTriggered = true;
//        }

        jsDirCurrent = joystkDirGet();

        if (jsDirPrev != jsDirCurrent)
        {
            jsDirPrev = jsDirCurrent;

            if (JOYST_UP == jsDirCurrent)
            {
                printf("joystick pushed up.\n");


                //Display next mode
                if(currentDispMode == dispModeSound)
                    currentDispMode = dispModeAlarmArm;
                else
                    currentDispMode++;

                printf("...[JS]currentDispMode: %d.\n", currentDispMode);

            }
            else if(JOYST_DOWN == jsDirCurrent)	
            {
                printf("joystick pushed down.\n");
        
                //Display previous mode
                if(currentDispMode == dispModeAlarmArm)
                    currentDispMode = dispModeSound;
                else
                    currentDispMode--;

                printf("...[JS]currentDispMode: %d.\n", currentDispMode);
            }
            else if(JOYST_PUSH == jsDirCurrent)
            {
                printf("joystick pushed center.\n");
                
                if(alarmTriggered)
                {
                    //Turn off the buzzer and disable the alarm
                    alarmTriggered = false;
                    alarmStateArm  = false;
                    
                    printf("...[JS]alarmStateArm turned off.\n");				
                }
                else if(currentDispMode == dispModeAlarmArm)
                {
                    //Push joystick to arm/unarm the alarm
                    if(alarmStateArm  == false)
                        alarmStateArm = true;
                    else
                        alarmStateArm = false;

                    printf("...[JS]alarmStateArm: %d.\n", alarmStateArm);
                }
            }
            else if(JOYST_RIGHT == jsDirCurrent)
            {
                printf("joystick pushed right.\n");
                
                if(currentDispMode == dispModeAlarmSound)
                {
                    pmwBuzzSelectNext(); 
                }
            }
            else if(JOYST_LEFT == jsDirCurrent)
            {
                printf("joystick pushed light.\n");

                if(currentDispMode == dispModeAlarmSound)
                {
                    pmwBuzzSelectPrv();
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
        }
        else if (alarmSleep) {
            alarmCntr++;
            alarmTriggered = false;

            if (alarmCntr >= BUZZER_ALARM_SLEEP_CNTR_MAX) 
            {
                //End sleep mode
                alarmSleep = false;

                //reset counter
                alarmCntr = 0;
            }
        } else {
            alarmCntr = 0;
        }

        //Delay
        nanosleep(&delay100ms, NULL);
	}
}


int main(int argc, char *argv[])
{
	int rt = 0;
//	pthread_t js_thread;
	pthread_t process_thread;

    //Turn on bus
    //fileWriteS(DEVICEs_SLOTS, "BB-I2C1");
    //fileWriteS(DEVICEs_SLOTS, "cape-universaln");

    if (rt == 0){
        rt = pmwBuzzInit();
    } else {
        printf("...[SysInit]Buzzer thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = digiDispInit();
    } else {
        printf("...[SysInit]4-digit diplay thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = tcpServerInit();
    } else {
        printf("...[SysInit]TCP server thread creation failed: %d\n", rt);
    }

    if (rt == 0){
        rt = joystkInit();
    } else {
        printf("...[SysInit]Joy stick thread creation failed: %d\n", rt);
    }


    //segDispInit();
    //
    //System start correctly
    if (rt == 0){
        printf("...[SysInit]System STARTED\n");
        sysInitStatusFlag = true;
    } else {
        return -1;
    }


/*
	rt = pthread_create(&js_thread, NULL,  (void *)&joystkInit, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}
*/

	rt = pthread_create(&process_thread, NULL,  (void *)&process, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

//	pthread_join( js_thread, NULL);
	pthread_join( process_thread, NULL);
	pthread_join( buzzer_thread, NULL);
	pthread_join( digiDisplay_thread, NULL);
//	pthread_join( tcpServerThreadId, NULL);

    tcpServerCleanup();
	return 0;
}