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

//Program state settings
bool stopping = false;
bool alarmTriggered = false;
bool alarmStateArm  = false;
int alarmBuzzMode   = 0;
int babySoundLevel  = 0;
int babyRoomTemp    = 0;
dispMode_t currentDispMode = dispModeAlarmArm;

void process(void)
{
//	static int i = 1;

	while(!stopping)
	{
		if(JOYST_UP == joystkDirection)
		{
			printf("joystick pushed up.\n");


			//Display next mode
			if(currentDispMode == dispModeSound)
				currentDispMode = dispModeAlarmArm;
			else
				currentDispMode++;

		}
		else if(JOYST_DOWN == joystkDirection)	
		{
			printf("joystick pushed down.\n");
	
			//Display previous mode
			if(currentDispMode == dispModeAlarmArm)
				currentDispMode = dispModeSound;
			else
				currentDispMode--;

		}
		else if(JOYST_PUSH == joystkDirection)
		{
			printf("joystick pushed center.\n");
			
			if(alarmTriggered)
			{
				//Turn off the buzzer and disable the alarm
				alarmTriggered = false;
				alarmStateArm  = false;				
			}
			else if(currentDispMode == dispModeAlarmArm)
			{
				//Push joystick to arm/unarm the alarm
				if(alarmStateArm  == false)
					alarmStateArm = true;
				else
					alarmStateArm = false;
			}
		}
		else if(JOYST_RIGHT == joystkDirection)
		{
			printf("joystick pushed right.\n");
			
			if(currentDispMode == dispModeAlarmSound)
			{

			}
		}
		else if(JOYST_LEFT == joystkDirection)
		{
			printf("joystick pushed light.\n");

			if(currentDispMode == dispModeAlarmSound)
			{

			}
		}

		nanosleep(&delay100ms, NULL);

	}
}


int main(int argc, char *argv[])
{
	int rt;
	pthread_t js_thread;
	pthread_t process_thread;

	//pmwBuzzInit();
	digiDispInit();
	tcpServerInit();


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