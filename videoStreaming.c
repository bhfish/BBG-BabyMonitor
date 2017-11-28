#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "watchDog.h"

static char* command = "./video/captureVideo -F -o -c0|ffmpeg -vcodec mjpeg -i pipe:0 -f mjpeg udp://127.0.0.1:1234";
static pthread_t video_thread;
static pid_t child_pid;

void Video_stopStreaming();
_Bool Video_startStreaming(void);

/*
* Stop video streaming
*/
void Video_stopStreaming()
{
	void *videoThreadExitStatus;
	kill(child_pid, SIGTERM);
	waitpid(child_pid, NULL, 0);

	if(pthread_join( video_thread, &videoThreadExitStatus)!=0)
	{
		printf("[ERROR] failed to join with terminated video thread failed reason: %s\n", strerror(errno));
    	};

	if (videoThreadExitStatus != PTHREAD_CANCELED)
    	{
		// bad things happened...
		printf("[ERROR] abnormal termination state of video thread\n");
		printf("try to send a cancel request and wish video thread can normally be terminated...\n");

		if(pthread_cancel(video_thread) != 0)
		{
		    printf("[ERROR] failed to send a cancel request to video thread failed reason: %s\n", strerror(errno));
		}
    	}

}

/*
* fork child process to execute command to stream video via UDP.
*/
static void* startStreamVideo()
{
	int rt=0;

	int watchDogRefID, watchDogTimer;
    	_Bool wasRegistrationSuccess;

	wasRegistrationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);
    	watchDogTimer = WatchDog_getWatchDogTimer()/2 - 1;

	if( (child_pid = fork()) == -1){
		printf("[ERROR] failed to fork child process for video streaming: %s\n", strerror(errno));
	}


	if(child_pid==0)
	{
		if (execl("/bin/sh", "sh", "-c", command, (char *) 0) == -1) {
			printf("[ERROR] failed to run videoCapture\n");
			exit(EXIT_FAILURE);
		}
	}else if(child_pid>0)
	{
		while(rt==0){
			// If child is running, kick the watch dog
			if (wasRegistrationSuccess) {           			
            			WatchDog_kickWatchDog(watchDogRefID);
        		}
			
			sleep(watchDogTimer);
			rt = waitpid(child_pid, NULL, WNOHANG);
		}
		if(rt==-1){
			printf("[ERROR] check videoCapture status error\n");
		}else{
			printf("[ERROR] videoCapture exited unexpectedly\n");
		}
	}

	pthread_exit(PTHREAD_CANCELED);

}

/*
* Start video streaming
*/
_Bool Video_startStreaming(void)
{
	int rt;

	//Start new thread
	rt = pthread_create(&video_thread, NULL,  (void *)&startStreamVideo, NULL);
    if( rt )
	{
		printf("Video Thread creation failed: %d\n", rt);
		return false;
	}

	return true;
}
