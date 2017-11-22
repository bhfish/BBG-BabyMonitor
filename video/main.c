#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

bool progRun = true;
char* command = "./capture -F -o -c0|ffmpeg -vcodec mjpeg -i pipe:0 -f mjpeg udp://127.0.0.1:1234";

/*
* Stop all tasks
*/
void programExit()
{
	progRun = false;
}

void* capture_video()
{
	system(command);
}


int main(int argc, char *argv[])
{
	int rt;
	pthread_t video_thread;
	
	//Start new thread
	rt = pthread_create(&video_thread, NULL,  (void *)&capture_video, NULL);
    	if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}


	pthread_join( video_thread, NULL);


	return 0;
}
