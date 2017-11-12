#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SERVER_IP "192.168.7.1"
#define SERVER_PORT_NUMBER 1234
#define MAX_BYTES_PER_PACKET 508

static int socketFd;
static struct sockaddr_in serverAddress;

_Bool WaveStreamer_startStreaming() {
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

void WaveStreamer_stopStreaming() {
	close(socketFd);
}

_Bool WaveStreamer_sendBuffer(void* buffer, int bufferSize) {
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

	return true;
}