#include <stdbool.h>
#include "buzzer.h"
#include "pmw.h"
#include "util.h"
#include "alarmMonitor.h"
#include "watchDog.h"

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
#define SONG_1_NODE_TOTAL_NUM 25


int song0[SONG_0_NODE_TOTAL_NUM][2] = {
							{BUZZ_SOUND_NODE_DO, 500},
							{BUZZ_SOUND_NODE_DO, 500},
							{BUZZ_SOUND_NODE_SO, 500},
							{BUZZ_SOUND_NODE_SO, 500},
							{BUZZ_SOUND_NODE_RA, 500},
							{BUZZ_SOUND_NODE_RA, 500},
							{BUZZ_SOUND_NODE_SO, 500},
							{BUZZ_SOUND_NODE_NONE, 500},
							{BUZZ_SOUND_NODE_FA, 500},
							{BUZZ_SOUND_NODE_FA, 500},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_DO, 500}
						 };

int song1[SONG_1_NODE_TOTAL_NUM][2] = {
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_MI, 1000},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_MI, 1000},
							{BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_SO, 500},
							{BUZZ_SOUND_NODE_DO, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_MI, 1000},
							{BUZZ_SOUND_NODE_FA, 500},
							{BUZZ_SOUND_NODE_FA, 500},
							{BUZZ_SOUND_NODE_FA, 250},
							{BUZZ_SOUND_NODE_FA, 500},
                            {BUZZ_SOUND_NODE_MI, 500},
                            {BUZZ_SOUND_NODE_MI, 500},
                            {BUZZ_SOUND_NODE_MI, 250},
                            {BUZZ_SOUND_NODE_MI, 250},
                            {BUZZ_SOUND_NODE_MI, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_DO, 500},
							{BUZZ_SOUND_NODE_RE, 500},
							{BUZZ_SOUND_NODE_SO, 1000}
						 };

typedef int songArr[2];
typedef struct{
	songArr *node;
	int nodeTotalNum;
}buzzNode_t;


static buzzNode_t songList[2];
static pthread_t buzzer_thread;
static _Bool buzzLoop;
static int buzzPeriodList[BUZZ_MODE_TOTAL];


int pmwBuzzConfigExport(void)
{
	int res = 0;

	res = fileWriteD(PMW_CONFIG_PATH(PMW_CHIP_NUM_BUZZ), 0);

	return res;
}

int Buzzer_turnOn(void)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_ENABLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), 1);

	return res;
}

int Buzzer_turnOff(void)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_ENABLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), 0);

	return res;
}

int Buzzer_setValue(int period, int cycle)
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

	return res;
}

int Buzzer_setSoundMode(int mode)
{
	int res = 0;

	if((mode < 0) || (mode >= BUZZ_MODE_TOTAL)){
		printf("ERROR: Buzzer mode %d is not supported.\n", mode);
		return -1;
	}

	res = Buzzer_setValue(buzzPeriodList[mode], (buzzPeriodList[mode])/2);

	return res;
}

int Buzzer_setDefaultMode(void)
{
	int res = 0;

	res = Buzzer_setSoundMode(0);

	return res;
}


int pmwBuzzPlay(songArr* node, int pos)
{
	int res = 0;

	res = Buzzer_setValue(node[pos][0], (node[pos][0])/2);
	Buzzer_turnOn();

    sleep_msec(node[pos][1]);

	Buzzer_turnOff();

	return res;
}

void Buzzer_slectPrevMode(void)
{
    int buzzMode = AlarmMonitor_getAlarmBuzzMode();

    buzzMode--;
    if (buzzMode < 0){
        buzzMode = (SONGS_TOTAL - 1);
    }
    AlarmMonitor_setAlarmBuzzMode(buzzMode);
}

void Buzzer_slectNextMode(void)
{
    int buzzMode = AlarmMonitor_getAlarmBuzzMode();

    buzzMode++;
    if (buzzMode >= SONGS_TOTAL){
        buzzMode = 0;
    }
    AlarmMonitor_setAlarmBuzzMode(buzzMode);
}

void pmwBuzzTask(void)
{
	int pos  = 0;
	int songMode;
    int watchDogRefID;
    _Bool wasRegistrationSuccess;

    wasRegistrationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);
	songMode = AlarmMonitor_getAlarmBuzzMode();

	while(!AlarmMonitor_isStopping()){
		if(AlarmMonitor_getAlarmTrigger() && AlarmMonitor_getAlarmArm()){
            int buzzMode = AlarmMonitor_getAlarmBuzzMode();

			//Check if the alarm sound is changed
			if(songMode != buzzMode){
				songMode = buzzMode;
				pos = 0;
			}

			//Play pre-defined buzzer sound
			pmwBuzzPlay(songList[songMode].node, pos);

			pos++;
			if(pos == songList[songMode].nodeTotalNum)
				pos = 0;

		}else{
			Buzzer_turnOff();
		}

		if (wasRegistrationSuccess) {
			WatchDog_kickWatchDog(watchDogRefID);
		}
		
		sleep_msec(100);
	}
}

void Buzzer_cleanUp(void)
{
    pthread_join( buzzer_thread, NULL);
}

int Buzzer_init(void)
{
	int res = 0;

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
    if( res ){
		printf("Thread buzzer creation failed: %d\n", res);
		return -1;
	}

	return res;
}
