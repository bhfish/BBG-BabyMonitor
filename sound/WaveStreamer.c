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
#include <time.h>

static _Bool initializeSocket();
static void* waveStreamer(void* args);
static _Bool sendSegments();
static _Bool sendPcmSegment(short* segment, int segmentSize);
static void stopStreaming();
static _Bool shouldStopStreaming();

#define SERVER_IP "192.168.7.1"
#define SERVER_PORT_NUMBER 1234
#define MAX_BYTES_PER_PACKET 16384
#define INITIAL_NUMBER_OF_PCM_SEGMENTS 300

static int socketFd;
static struct sockaddr_in serverAddress;

static sem_t bufferFull;
static pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
static short* pcmSegments[INITIAL_NUMBER_OF_PCM_SEGMENTS];
static int pcmSegmentSize = 0;
static int currentPcmSegment = 0;
static int numberOfSegmentsToBuffer = 0;

static pthread_t streamingThread;
static pthread_mutex_t stopStreamingMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopStreamingFlag = false;

_Bool WaveStreamer_startStreaming(int segmentSize) {
	if (pthread_create(&streamingThread, NULL, &waveStreamer, NULL) != 0) {
        printf("Error: failed to create new listener thread\n");
        return false;
    }

    pcmSegmentSize = segmentSize;
    numberOfSegmentsToBuffer = INITIAL_NUMBER_OF_PCM_SEGMENTS;
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

void WaveStreamer_setSegment(short* buffer) {
	pthread_mutex_lock(&bufferMutex);
	{
		if (currentPcmSegment == numberOfSegmentsToBuffer) {
			sem_post(&bufferFull);
		} else {
			pcmSegments[currentPcmSegment] = malloc(pcmSegmentSize * sizeof(short));
			memcpy(pcmSegments[currentPcmSegment], buffer, pcmSegmentSize);
			currentPcmSegment++;
		}
	}
	pthread_mutex_unlock(&bufferMutex);
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

	if (!initializeSocket()) {
		printf("Error: unable to initialize UDP socket\n");

        pthread_exit(PTHREAD_CANCELED);
	}

	while (!shouldStopStreaming()) {
		if (!sendSegments()) {
			printf("Error: unable to send segments\n");

            //pthread_exit(PTHREAD_CANCELED);
		}
	}

	pthread_exit(PTHREAD_CANCELED);
}

static short** getSegments() {
	short** segments = malloc(numberOfSegmentsToBuffer * sizeof(short *));
	sem_wait(&bufferFull);
	pthread_mutex_lock(&bufferMutex);
	{
		for (int i = 0 ; i < numberOfSegmentsToBuffer ; i++) {
			segments[i] = malloc(pcmSegmentSize * sizeof(short));
			memcpy(segments[i], pcmSegments[i], pcmSegmentSize);
		}

		numberOfSegmentsToBuffer = INITIAL_NUMBER_OF_PCM_SEGMENTS / 3;
		currentPcmSegment = 0;
	}
	pthread_mutex_unlock(&bufferMutex);

	return segments;
}

static _Bool sendSegments() {
	int segmentsCount = numberOfSegmentsToBuffer;
	short** segments = getSegments();
	for (int i = 0 ; i < segmentsCount ; i++) {
		sendPcmSegment(segments[i], pcmSegmentSize);
	}

	free(segments);
	return true;
}

static _Bool sendPcmSegment(short* segment, int segmentSize) {
	int packetsToSend = (segmentSize % MAX_BYTES_PER_PACKET == 0) ? segmentSize / MAX_BYTES_PER_PACKET : (segmentSize / MAX_BYTES_PER_PACKET) + 1;
	int packetSize = (segmentSize / MAX_BYTES_PER_PACKET == 0) ?  segmentSize % MAX_BYTES_PER_PACKET : MAX_BYTES_PER_PACKET;
	int finalPacketSize = (segmentSize % MAX_BYTES_PER_PACKET == 0) ? MAX_BYTES_PER_PACKET : segmentSize % MAX_BYTES_PER_PACKET;
	for (int i = 0; i < packetsToSend ; i++) {
		int sendPacketSize = (i == (packetsToSend - 1)) ? packetSize : finalPacketSize;
		if (sendto(socketFd, segment + (i * packetSize), sendPacketSize, MSG_DONTWAIT, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
			printf("Error: sending segment\n");
			return false;
		}

		nanosleep((const struct timespec[]){{0, 125000000L}}, NULL);
	}

	free(segment);
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