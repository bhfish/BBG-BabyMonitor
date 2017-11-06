#include <stdbool.h>
#include "parent_buzzer.h"
#include "parent_pmw.h"
#include "parent_publicFunc.h"
#include "parent_process.h"

#define BUZZ_MODE_TOTAL        10
#define BUZZ_SOUND_IDX_DEFAULT  0

#define BUZZ_SOUND_IDX_NONE     0
#define BUZZ_SOUND_IDX_DO       1
#define BUZZ_SOUND_IDX_RE       2
#define BUZZ_SOUND_IDX_MI       3
#define BUZZ_SOUND_IDX_FA       4
#define BUZZ_SOUND_IDX_SO       5
#define BUZZ_SOUND_IDX_RA       6
#define BUZZ_SOUND_IDX_SI       7

#define BUZZ_SOUND_NODE_NONE     0
#define BUZZ_SOUND_NODE_DO       3822630
#define BUZZ_SOUND_NODE_RE       3405994
#define BUZZ_SOUND_NODE_MI       3033980
#define BUZZ_SOUND_NODE_FA       2863688
#define BUZZ_SOUND_NODE_SO       2551020
#define BUZZ_SOUND_NODE_RA       2272727
#define BUZZ_SOUND_NODE_SI       2025111

#define SONGS_TOTAL           2
#define SONG_0_NODE_TOTAL_NUM 15
#define SONG_1_NODE_TOTAL_NUM 15


int song0[SONG_0_NODE_TOTAL_NUM][2] = {
							{BUZZ_SOUND_NODE_DO, 1},
							{BUZZ_SOUND_NODE_DO, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_RA, 1},
							{BUZZ_SOUND_NODE_RA, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_NONE, 1},
							{BUZZ_SOUND_NODE_FA, 1},
							{BUZZ_SOUND_NODE_FA, 1},
							{BUZZ_SOUND_NODE_MI, 1},
							{BUZZ_SOUND_NODE_MI, 1},
							{BUZZ_SOUND_NODE_RE, 1},
							{BUZZ_SOUND_NODE_RE, 1},
							{BUZZ_SOUND_NODE_DO, 1}
						 };

int song1[SONG_1_NODE_TOTAL_NUM][2] = {
							{BUZZ_SOUND_NODE_DO, 1},
							{BUZZ_SOUND_NODE_DO, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_RA, 1},
							{BUZZ_SOUND_NODE_RA, 1},
							{BUZZ_SOUND_NODE_SO, 1},
							{BUZZ_SOUND_NODE_NONE, 1},
							{BUZZ_SOUND_NODE_FA, 1},
							{BUZZ_SOUND_NODE_FA, 1},
							{BUZZ_SOUND_NODE_MI, 1},
							{BUZZ_SOUND_NODE_MI, 1},
							{BUZZ_SOUND_NODE_RE, 1},
							{BUZZ_SOUND_NODE_RE, 1},
							{BUZZ_SOUND_NODE_DO, 1}
						 };

typedef int songArr[2];

typedef struct
{
	songArr *node;
	int nodeTotalNum;
}buzzNode_t;

buzzNode_t songList[2];

pthread_t buzzer_thread;

static bool buzzLoop;


//buzz_t buzzList[BUZZ_MODE_TOTAL];

int buzzPeriodList[BUZZ_MODE_TOTAL];


int pmwBuzzConfigExport(void)
{
	int res = 0;

	res = fileWriteD(PMW_CONFIG_PATH(PMW_CHIP_NUM_BUZZ), 0);

	return res;
}

int pmwBuzzOn(void)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_ENABLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), 1);

	return res;
}

int pmwBuzzOff(void)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_ENABLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), 0);

	return res;
}

int pmwBuzzValueSet(int period, int cycle)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_CYCLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), 0);

	if(0 == res)
	{
		res = fileWriteD(PMW_SYS_FS_PERIOD(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), period);
	}

	if(0 == res)
	{
		res = fileWriteD(PMW_SYS_FS_CYCLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), cycle);
	}

//	if(0 == res)
//	{
//		res = pmwBuzzOn();
//	}

	return res;
}

int pmwBuzzSound(int mode)
{
	int res = 0;

	if((mode < 0) || (mode >= BUZZ_MODE_TOTAL))
	{
		printf("ERROR: Buzzer mode %d is not supported.\n", mode);
		return -1;
	}

	res = pmwBuzzValueSet(buzzPeriodList[mode], (buzzPeriodList[mode])/2);

	return res;
}

int pmwBuzzModeDefault(void)
{
	int res = 0;

	res = pmwBuzzSound(0);

	return res;
}


int pmwBuzzPlay(songArr* node, int pos)
{
	int res = 0;

	res = pmwBuzzValueSet(node[pos][0], (node[pos][0])/2);
	pmwBuzzOn();

	//Delay after playing each node
	for(int i = 0; i < node[pos][1]; i++)
	{
		nanosleep(&delay500ms, NULL);
	}

	pmwBuzzOff();

	return res;
}

void pmwBuzzTask(void)
{
	static int pos  = 0;
	static int songMode;

	songMode = alarmBuzzMode;

	while(!stopping)
	{
		if(alarmTriggered && alarmStateArm)
		{
			//Check if the alarm sound is changed
			if(songMode != alarmBuzzMode)
			{
				songMode = alarmBuzzMode;
				pos = 0;
			}
			
			//Play pre-defined buzzer sound
			pmwBuzzPlay(songList[songMode].node, pos);

			pos++;
			if(pos == songList[songMode].nodeTotalNum)
				pos = 0;

		}
		else
		{
			pmwBuzzOff();
		}

		nanosleep(&delay100ms, NULL);
	}

}

int pmwBuzzLoop(void)
{
	int res = 0;

	pmwBuzzSound(BUZZ_SOUND_IDX_DO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_DO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_SO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_SO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_RA);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_RA);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_SO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	
	pmwBuzzOff();
	nanosleep(&delay1s, NULL);

	
	pmwBuzzSound(BUZZ_SOUND_IDX_FA);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_FA);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);
	
	pmwBuzzSound(BUZZ_SOUND_IDX_MI);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_MI);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_RE);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_RE);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	pmwBuzzSound(BUZZ_SOUND_IDX_DO);
	pmwBuzzOn();
	nanosleep(&delay500ms, NULL);
	pmwBuzzOff();
	nanosleep(&delay100ms, NULL);

	return res;
}

int pmwBuzzInit(void)
{
	int res = 0;

	//Configure the CPU’s pin connected to the buzzer for use with PWM
	//TO-DO

	//Begin configuring the PWM functionality for the buzzer by exporting it
	//res = pmwBuzzConfigExport();

	//Load Buzzer sound mode
	buzzPeriodList[0] = 1000000;
	buzzPeriodList[1] = 3822630;
	buzzPeriodList[2] = 3405994;
	buzzPeriodList[3] = 3033980;
	buzzPeriodList[4] = 2863688;
	buzzPeriodList[5] = 2551020; 
	buzzPeriodList[6] = 2272727; 
	buzzPeriodList[7] = 2025111; 

	buzzLoop = false;


	//Initalize song list
	songList[0].node = song0;
	songList[0].nodeTotalNum = SONG_0_NODE_TOTAL_NUM;
	songList[1].node = song1;
	songList[1].nodeTotalNum = SONG_1_NODE_TOTAL_NUM;


	res = pthread_create(&buzzer_thread, NULL,  (void *)&pmwBuzzTask, NULL);
    if( res )
	{
		printf("Thread buzzer creation failed: %d\n", res);
		return -1;	
	}

	return res;
}
