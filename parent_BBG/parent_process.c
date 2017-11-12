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

//The maximum time buzzer sounds
#define BUZZER_ALARM_CNTR_MAX  600

//Program state settings
bool stopping = false;
bool alarmTriggered = false;
bool alarmStateArm  = false;
int alarmBuzzMode   = 0;
int babySoundLevel  = 0;
int babyRoomTemp    = 0;
dispMode_t currentDispMode = dispModeAlarmArm;


void setBbySoundLevel(int sound)
{
    babySoundLevel = sound;
}

int getBbySoundLevel(void)
{
    return babySoundLevel;
}

void process(void)
{
//	static int i = 1;

    static unsigned int alarmCntr = 0;

	while(!stopping)
	{
//        if (babySoundLevel > SOUND_ALARM_TRIGGER_THRESHOLD){
//            alarmTriggered = true;
//        }

        if (JOYST_UP == joystkDirection)
		{
			printf("joystick pushed up.\n");


			//Display next mode
			if(currentDispMode == dispModeSound)
				currentDispMode = dispModeAlarmArm;
			else
				currentDispMode++;

            printf("...[JS]currentDispMode: %d.\n", currentDispMode);

		}
		else if(JOYST_DOWN == joystkDirection)	
		{
			printf("joystick pushed down.\n");
	
			//Display previous mode
			if(currentDispMode == dispModeAlarmArm)
				currentDispMode = dispModeSound;
			else
				currentDispMode--;
            printf("...[JS]currentDispMode: %d.\n", currentDispMode);
		}
		else if(JOYST_PUSH == joystkDirection)
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
		else if(JOYST_RIGHT == joystkDirection)
		{
			printf("joystick pushed right.\n");
			
			if(currentDispMode == dispModeAlarmSound)
			{
                pmwBuzzSelectNext(); 
			}
		}
		else if(JOYST_LEFT == joystkDirection)
		{
			printf("joystick pushed light.\n");

			if(currentDispMode == dispModeAlarmSound)
			{
                pmwBuzzSelectPrv();
			}
		}

        if (alarmTriggered)
        {
            alarmCntr++;
            if (alarmCntr >= BUZZER_ALARM_CNTR_MAX)
            {
                alarmTriggered = false;
                alarmCntr = 0;
            }
        }
        else
        {
            alarmCntr = 0;
        }

        //Delay
        nanosleep(&delay100ms, NULL);
	}
}


int main(int argc, char *argv[])
{
	int rt;
	pthread_t js_thread;
	pthread_t process_thread;

    //Turn on bus
    //fileWriteS(DEVICEs_SLOTS, "BB-I2C1");
    //fileWriteS(DEVICEs_SLOTS, "cape-universaln");

	pmwBuzzInit();
	digiDispInit();
	tcpServerInit();
    //segDispInit();

	rt = pthread_create(&js_thread, NULL,  (void *)&joystkInit, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	rt = pthread_create(&process_thread, NULL,  (void *)&process, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	pthread_join( js_thread, NULL);
	pthread_join( process_thread, NULL);

	pthread_join( buzzer_thread, NULL);
	pthread_join( digiDisplay_thread, NULL);
	pthread_join( tcpServerThreadId, NULL);

	return 0;
}