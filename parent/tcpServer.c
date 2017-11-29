/**
 *
 * A simple server in the internet domain using TCP.The port
 * number is passed as an argument
 *
 * Some code are captured from
 * http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>      // errno
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include "alarmMonitor.h"
#include "tcpServer.h"
#include "monitorData.h"

/*
 * Define constant
 */
#define RX_BUFLEN   500
#define TX_BUFLEN   500
#define PORT  12345
#define MAX_NUM_CONNECTION  5

//const struct timespec delay100ms = {0, 100000000};

static int socketfd;
static int newsocketfd = 0;
static int client_lenth;
static struct sockaddr_in tcp_server;
static struct sockaddr_in tcp_client;
pthread_t tcpServerThreadId;

//static int tcpServerCmdParse(char* rx_buffer);
static int tcpServerCmdParse(char* rx_buffer, char* tx_buffer);
static void tcpServerTask(void);
static int tcpServerBindPort(void);
//static void test(void);
//static void TCPServer_cleanUp(void);

static void tcpServerTask(void)
{
	struct timeval timeSpan;
	timeSpan.tv_sec = 0;
	timeSpan.tv_usec = 100;

	char rx_buffer[RX_BUFLEN];
    char tx_buffer[TX_BUFLEN];
    fd_set activeFD, readFD;

    // WILSON, a bug here. have to explicitly set stopping to false; otherwise, sometimes the following codes won't run
    //stopping = false;
	//char tx_buffer[TX_BUFLEN];
	//int res;
	int numberOfConnection;
	int num_bytes;


	client_lenth = sizeof(tcp_client);

    /* Initialize the set of active sockets. */
    FD_ZERO (&activeFD);
    FD_SET (socketfd, &activeFD);

    // TCP server referenced from: http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
	while(!AlarmMonitor_isStopping())
	{
        char clientIPAddrName[100] = {0};
        readFD = activeFD;
		numberOfConnection = select(FD_SETSIZE, &readFD, NULL, NULL, &timeSpan);

		if(numberOfConnection>0)
		{
            /* Service all the sockets with input pending. */
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET (i, &readFD) ){
                    if (i == socketfd) {
                        /* Connection request on original socket. */
                        newsocketfd = accept(socketfd, (struct sockaddr *) &tcp_client, (socklen_t *)&client_lenth);
                        if(newsocketfd < 0){
                            printf("ERROR on accept");
                            break;
                        }

                        FD_SET (newsocketfd, &activeFD);
                    }
                    else {
                        /* Data arriving on an already-connected socket. */
                        //printf("I'm here s\n");
                        memset(rx_buffer, 0, sizeof(rx_buffer));
                        memset(tx_buffer, '\0', sizeof(tx_buffer));

                        num_bytes = read(i, rx_buffer, RX_BUFLEN);
                        //printf("numb: %d\n", num_bytes);

                        if(num_bytes > 0)
                        {
                            tcpServerCmdParse(rx_buffer, tx_buffer);
                            //printf("Here is the message: %s\n",rx_buffer);
                            if (strlen(tx_buffer) != 0){
                                inet_ntop(AF_INET, &tcp_client.sin_addr.s_addr, clientIPAddrName, INET_ADDRSTRLEN);
                                int numBytesWrite = write(i, tx_buffer, strlen(tx_buffer));
                                printf("...[TCP]Reply: %s, size %d to %s\n",tx_buffer, numBytesWrite, clientIPAddrName);
                            }
                        }
                    }
                }
            }
		}
	}
}

static int tcpServerBindPort(void)
{
	int res = 0;
	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (socketfd < 0)
        printf("ERROR opening socket");

    //bzero((char *) &tcp_server, sizeof(tcp_server));
	memset (&tcp_server, 0, sizeof(struct sockaddr_in));

	tcp_server.sin_family = AF_INET;
	tcp_server.sin_addr.s_addr = INADDR_ANY;
    tcp_server.sin_port = htons(PORT);


	if (bind(socketfd, (struct sockaddr *) &tcp_server,sizeof(tcp_server)) < 0)
        printf("ERROR on binding reason: %s", strerror(errno));

	listen(socketfd, MAX_NUM_CONNECTION);

	return res;

}

static int tcpServerCmdParse(char* rx_buffer, char* tx_buffer)
{
    int res = 0;
    const char DELIM[2] = ":;";
    char *token;

    printf("rx_buffer: %s\n", rx_buffer);

    if (rx_buffer[strlen(rx_buffer) - 1] != ';') {
        printf("ERROR: unexpected message format\n");

        return -1;
    }

    token = strtok(rx_buffer, DELIM);

    while (token != NULL) {

        if (strcmp(token, ALARM_TYPE) == 0) {
        	if (AlarmMonitor_getAlarmSleepStatus()) {
            	printf("...[TCP]Alarm in sleep mode, will not trigger alarm.\n");
             } else {
                AlarmMonitor_setAlarmTrigger(true);
             	printf("...[TCP]Alarm trigger received.\n");
             }

        }
        else if (strcmp(token, "getAlarmBBGStatus") == 0) {
			if (AlarmMonitor_getSysInitStatus()){
            	sprintf(tx_buffer, "%s:%s", token, "Active");
            } else {
            	sprintf(tx_buffer, "%s:%s", token, "Inactive");
			}

        }
        else if(strcmp("close", token) == 0)
        {
            AlarmMonitor_stopProg();
            //TCPServer_cleanUp();
            printf("Now close the program\n");
        }
        else {
            if (strcmp(token, TEMPERATURE_TYPE_DATA) == 0) {
                token = strtok(NULL, DELIM);
                int roomTemp = atoi(token);

                AlarmMonitor_setBbyRoomTemp(roomTemp);
                printf("Temperature is %d\n", roomTemp);
            }
            else if (strcmp(token, SOUND_TYPE_DATA) == 0) {
                token = strtok(NULL, DELIM);
                int sound = atoi(token);

                AlarmMonitor_setBbySoundLevel(sound);
                printf("SOUND is %d\n", sound);
            }
        }

        // move to next token
        token = strtok(NULL, DELIM);
    }

	return res;
}

void TCPServer_cleanUp(void)
{
	close(socketfd);
	close(newsocketfd);

    pthread_join(tcpServerThreadId, NULL);
}

int TCPServer_init(void)
{
	int res= 0;

	tcpServerBindPort();

	res = pthread_create(&tcpServerThreadId, NULL,  (void *)&tcpServerTask, NULL);

    if( res ){
		printf("Thread creation failed: %d\n", res);
		return -1;
	}

	return res;
}


