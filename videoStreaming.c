#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h> 

//bool progRun = true;
static char* command = "./video/captureVideo -F -o -c0|ffmpeg -vcodec mjpeg -i pipe:0 -f mjpeg udp://192.168.7.1:1234";
static pthread_t video_thread;
//static _Bool stopStreaming = false;
static pid_t child_pid;
void Video_stopStreaming();
_Bool Video_startStreaming(void);

/*
* Stop video streaming
*/
void Video_stopStreaming()
{
	void *videoThreadExitStatus;
	//stopStreaming = true;
	kill(child_pid, SIGTERM);
	
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
* execute command to stream video via UDP./
*/
static void* startStreamVideo()
{
	//system(command);
	if( (child_pid = fork()) == -1)
	{
		printf("[ERROR] failed to fork child process for video streaming: %s\n", strerror(errno));
	}

	if(child_pid>0)
	{
		execl("/bin/sh", "sh", "-c", command, (char *) 0);
	}else if(child_pid==0)
	{
		wait(NULL);
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
		printf("Thread creation failed: %d\n", rt);
		return false;	
	}
	
	return true;
}
