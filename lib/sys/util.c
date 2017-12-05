#include "util.h"

const struct timespec delay1s = {1, 0};
const struct timespec delay400ns = {0, 400};
const struct timespec delay1us   = {0, 100000};
const struct timespec delay1ms   = {0, 1000000};
const struct timespec delay100ms = {0, 100000000};
const struct timespec delay500ms = {0, 500000000};


void sleep_usec(long usec)
{
	struct timespec sleep_time;
	sleep_time.tv_sec = (usec / 1000000);
	sleep_time.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&sleep_time, NULL);
}

void sleep_msec(long msec)
{
	sleep_usec(msec * 1000);
}

void sleep_sec(long sec)
{
	struct timespec sleep_time;
	sleep_time.tv_sec = sec;
	sleep_time.tv_nsec = 0;
	nanosleep(&sleep_time, NULL);
}

int fileWriteD(char* filePath, int value)
{
	FILE *pfile = NULL;
	int res = 0;

	pfile = fopen(filePath, "w");

	if (pfile == NULL) {
		printf("ERROR: Unable to open %s for write.\n", filePath);
		return -1;
	}

	if (fprintf(pfile, "%d", value) <= 0) {
		printf("ERROR: Writing data to %s.\n", filePath);
		res = -1;
	}

	fclose(pfile);
	return res;
}


int fileWriteS(char* filePath, char* value)
{
	FILE *pfile = NULL;
	int res = 0;

	pfile = fopen(filePath, "w");

	if (pfile == NULL) {
		printf("ERROR: Unable to open %s for write.\n", filePath);
		return -1;
	}

	if (fprintf(pfile, "%s", value) <= 0) {
		printf("ERROR: Writing data to %s.\n", filePath);
		res = -1;
	}

	fclose(pfile);
	return res;
}


int fileReadD(char* filePath)
{
	FILE *pfile = NULL;
	int value;

	pfile = fopen(filePath, "r");

	if (pfile == NULL) {
		printf("ERROR: Unable to open %s for read.\n", filePath);
		return -1;
	}

	//Get value and conver char to int
	value = fgetc(pfile) - '0';

	fclose(pfile);
	return value;
}