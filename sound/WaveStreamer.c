#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>

static _Bool initializeSocket();
static void* waveStreamer(void* args);
static _Bool sendBuffer();
static void stopStreaming();
static _Bool shouldStopStreaming();

#define SERVER_IP "192.168.7.1"
#define SERVER_PORT_NUMBER 1234
#define MAX_BYTES_PER_PACKET 16384

static int socketFd;
static struct sockaddr_in serverAddress;

static sem_t bufferFull;
static sem_t bufferEmpty;
static short* pcmBuffer = NULL;
static int pcmBufferSize = 0;

static pthread_t streamingThread;
static pthread_mutex_t stopStreamingMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopStreamingFlag = false;

_Bool WaveStreamer_startStreaming() {
	if (pthread_create(&streamingThread, NULL, &waveStreamer, NULL) != 0) {
        printf("Error: failed to create new listener thread\n");
        return false;
    }

    return true;
}

void WaveStreamer_stopStreaming() {
	stopStreaming();

	void* streamingThreadExitStatus;
    if (pthread_join(streamingThread, &streamingThreadExitStatus) != 0) {
        printf("Error: failed to join listener thread\n");
    }

    if (streamingThreadExitStatus != PTHREAD_CANCELED) {
        printf("Error: thread has not been canceled, attempting to terminate thread\n");
        if (pthread_cancel(streamingThread) != 0) {
            printf("Error: thread failed to cancel\n");
        }
    }

	close(socketFd);
}

void WaveStreamer_setBuffer(void* buffer, int bufferSize) {
	sem_wait(&bufferEmpty);
	{
		pcmBufferSize = bufferSize;
		pcmBuffer = (short *) malloc(pcmBufferSize);
		memcpy(pcmBuffer, buffer, pcmBufferSize);
	}
	sem_post(&bufferFull);
}

static _Bool initializeSocket() {
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(SERVER_PORT_NUMBER);

	socketFd = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketFd == -1) {
		printf("Error: failed to create socket because of: %s\n", strerror(errno));
		return false;
	}

	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
		printf("Error: failed to set socket as 0_NONBLOCK because of: %s\n", strerror(errno));
		close(socketFd);
		return false;
	}

	int flags = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, &flags, sizeof(flags))) {
		printf("Error: failed to set socket operation because of: %s\n", strerror(errno));
		return false;
	}

	return true;
}

static void* waveStreamer(void* args) {
	sem_init(&bufferFull, 0, 0);
	sem_init(&bufferEmpty, 0, 1);

	if (!initializeSocket()) {
		printf("Error: unable to initialize UDP socket\n");

        pthread_exit(PTHREAD_CANCELED);
	}

	while (!shouldStopStreaming()) {
		if (!sendBuffer()) {
			printf("Error: unable to send buffer\n");

            pthread_exit(PTHREAD_CANCELED);
		}
	}

	pthread_exit(PTHREAD_CANCELED);
}

static _Bool sendBuffer() {
	short* buffer = NULL;
	int bufferSize = 0;
	sem_wait(&bufferFull);
    {
    	bufferSize = pcmBufferSize;
    	buffer = malloc(bufferSize);
    	memcpy(buffer, pcmBuffer, bufferSize);
    	free(pcmBuffer);
    }
    sem_post(&bufferEmpty);

    int packetsToSend = (bufferSize % MAX_BYTES_PER_PACKET == 0) ? bufferSize / MAX_BYTES_PER_PACKET : (bufferSize / MAX_BYTES_PER_PACKET) + 1;
	int packetSize = (bufferSize / MAX_BYTES_PER_PACKET == 0) ? MAX_BYTES_PER_PACKET : bufferSize % MAX_BYTES_PER_PACKET;
	int finalPacketSize = (bufferSize % MAX_BYTES_PER_PACKET == 0) ? MAX_BYTES_PER_PACKET : bufferSize % MAX_BYTES_PER_PACKET;

	for (int i = 0; i < packetsToSend ; i++) {
		int sendPacketSize = (i == (packetsToSend - 1)) ? packetSize : finalPacketSize;
		if (sendto(socketFd, buffer + (i * packetSize), sendPacketSize, MSG_NOSIGNAL, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
			printf("Error: sending buffer\n");
			return false;
		}
	}

	free(buffer);
	return true;
}

static void stopStreaming() {
	pthread_mutex_lock(&stopStreamingMutex);
    {
        stopStreamingFlag = true;
    }
    pthread_mutex_unlock(&stopStreamingMutex);
}

static _Bool shouldStopStreaming() {
	_Bool stopStreaming = false;
	pthread_mutex_lock(&stopStreamingMutex);
    {
        stopStreaming = stopStreamingFlag;
    }
    pthread_mutex_unlock(&stopStreamingMutex);

    return stopStreaming;
}