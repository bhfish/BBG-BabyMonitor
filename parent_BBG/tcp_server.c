/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "parent_process.h"
#include "tcp_server.h"

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
static bool finished = false;
static struct sockaddr_in tcp_server;
static struct sockaddr_in tcp_client;
pthread_t tcpServerThreadId;

//static int tcpServerCmdParse(char* rx_buffer);
static int tcpServerCmdParse(char* rx_buffer, char* tx_buffer);
static void tcpServerTask(void);
static int tcpServerBindPort(void);
//static void test(void);
//static void tcpServerCleanup(void); 

static void tcpServerTask(void)
{
	struct timeval timeSpan;
	//timeSpan.tv_sec =0;
	//timeSpan.tv_usec = 100;

	char rx_buffer[RX_BUFLEN];
    char tx_buffer[TX_BUFLEN];
	fd_set socketFileDescirptor;

	//char tx_buffer[TX_BUFLEN];
	//int res;
	int numberOfConnection;
	int num_bytes;
	//int pid;

	client_lenth = sizeof(tcp_client);
	
	
	while(!stopping)
	{
		finished = false;
		FD_ZERO(&socketFileDescirptor);
		FD_SET(socketfd, &socketFileDescirptor);
		timeSpan.tv_sec =0;
		timeSpan.tv_usec = 100;
		//newsocketfd = accept(socketfd, (struct sockaddr *) &tcp_client, &client_lenth);
		numberOfConnection = select(socketfd + 1, &socketFileDescirptor, NULL, NULL, &timeSpan);
		//printf("number of connection%d\n",numberOfConnection);
		
		if(numberOfConnection>0)
		{

			printf("number of connection%d\n",numberOfConnection);
			newsocketfd = accept(socketfd, (struct sockaddr *) &tcp_client, (socklen_t *)&client_lenth);
			if(newsocketfd < 0){
				printf("ERROR on accept");
			}
			 while(!finished){
			 	//printf("I'm here s\n");
				memset(rx_buffer, 0, sizeof(rx_buffer));
                memset(tx_buffer, '\0', sizeof(tx_buffer));

                num_bytes = read(newsocketfd, rx_buffer, RX_BUFLEN);
                //printf("numb: %d\n", num_bytes);

				if(num_bytes > 0)
				{
                    tcpServerCmdParse(rx_buffer, tx_buffer); 
					//printf("Here is the message: %s\n",rx_buffer);
				}
				else{
					finished = true;
				}

                if (strlen(tx_buffer) != 0){
                    write(newsocketfd, tx_buffer, strlen(tx_buffer));
                    printf("...[TCP]Reply: %s, strlen %d \n",tx_buffer, strlen(tx_buffer));
                }
			}
		
		//if(num_bytes < 0) printf("ERROR reading from socket");
		
		//num_bytes = write(newsocketfd, "I got your message\n", 19);
		
        //if(num_bytes < 0) printf("ERROR writing from socket");
        //close(newsocketfd);
        	//nanosleep(&delay100ms, NULL);

		}
	//printf("TCP SERVER CLOSED\n");
	}	
		
	 /*while (!stopping) {
         newsocketfd = accept(socketfd, (struct sockaddr *) &tcp_client, &client_lenth);
	
         if (newsocketfd < 0) 
            printf("ERROR on accept");
         pid = fork();
         if (pid < 0)
             printf("ERROR on accept");
         if (pid == 0)  {
             close(socketfd);
             memset(rx_buffer, 0, sizeof(rx_buffer));
  		//memset(tx_buffer, '\0', sizeof(tx_buffer));
			num_bytes = read(newsocketfd, rx_buffer, RX_BUFLEN);
		//tcpServerCmdParse(rx_buffer);
			printf("Here is the message: %s\n",rx_buffer);

			if(num_bytes < 0) printf("ERROR reading from socket");
		
			num_bytes = write(newsocketfd, "I got your message\n", 19);
		
        	if(num_bytes < 0) printf("ERROR writing from socket");
            /// dostuff(newsockfd);
             exit(0);
         }
         else close(newsocketfd);
     } */
	
 
     
}
/*static void test(void)
{
	char rx_buffer[RX_BUFLEN];
	//char tx_buffer[TX_BUFLEN];
	int num_bytes;

	client_lenth = sizeof(tcp_client);
	newsocketfd = accept(socketfd, (struct sockaddr *) &tcp_client, &client_lenth);
	
	if(newsocketfd < 0)
		printf("ERROR on accept");

	memset(rx_buffer, 0, sizeof(rx_buffer));
  		//memset(tx_buffer, '\0', sizeof(tx_buffer));
	num_bytes = read(newsocketfd, rx_buffer, RX_BUFLEN);
	printf("Here is the message: %s\n",rx_buffer);

	if(num_bytes < 0) printf("ERROR reading from socket");
		
	num_bytes = write(newsocketfd, "I got your message", 18);
		
    if(num_bytes < 0) printf("ERROR writing from socket");

}*/

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
            printf("ERROR on binding");
	
	listen(socketfd, MAX_NUM_CONNECTION);
	
	return res;
	
}

static int tcpServerCmdParse(char* rx_buffer, char* tx_buffer)
{
	int res = 0;
    char* buf_tokens[2];

    if ((buf_tokens[0] = strtok(rx_buffer,": \n\t\0")) != NULL){
   		buf_tokens[1] = strtok(NULL, " \n\t\0");
    }
    else{
        return -1;
    }

	if(strcmp("temperature", buf_tokens[0]) == 0)
	{
		babyRoomTemp = atoi(buf_tokens[1]);
        printf("Temperature is %d\n", babyRoomTemp);
	}
	else if(strcmp("sound", buf_tokens[0]) == 0)
	{
		babySoundLevel = atoi(buf_tokens[1]);
        setBbySoundLevel(babySoundLevel);
        printf("SOUND is %d\n", babySoundLevel);
	}
	else if(strcmp("alarm", buf_tokens[0]) == 0)
	{
        if (getAlarmSleepStatus()) {
            printf("...[TCP]Alarm in sleep mode, will not trigger alarm.\n");
        } else {
            alarmTriggered = true;
            printf("...[TCP]Alarm trigger received.\n");
        }
	}
	else if(strcmp("getParentBBGStatus", buf_tokens[0]) == 0)
	{
        if (getSysInitStatus()){
            strcpy(tx_buffer, "Active");
        } else {
            strcpy(tx_buffer, "Inactive");
        }
        printf("...[TCP]Reply parent BBG status: %s.\n", tx_buffer);
	}
	else if(strcmp("armed", buf_tokens[0]) == 0)
	{
        alarmStateArm = true;
        printf("...[TCP]Alarm armed.\n");
	}
	else if(strcmp("close", buf_tokens[0]) == 0)
	{
		stopping = true;
		tcpServerCleanup();
		printf("Now close the program\n");
	}

	return res;
}

void tcpServerCleanup(void)
{
	close(socketfd);
	close(newsocketfd);

    pthread_join(tcpServerThreadId, NULL);
}

int tcpServerInit(void)
{
	int res= 0;

	tcpServerBindPort();

	res = pthread_create(&tcpServerThreadId, NULL,  (void *)&tcpServerTask, NULL);

    if( res )
	{
		printf("Thread creation failed: %d\n", res);
		return -1;	
	}

	return res;

	//test();
	//tcpServerCleanup();
}


