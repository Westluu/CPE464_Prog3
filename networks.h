
// 	Writen - HMS April 2017
//  Supports TCP and UDP - both client and server


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "networks.h"
#include "gethostbyname.h"

#define LISTEN_BACKLOG 10
#define CRC_ERROR -1

typedef struct Connection {
    uint32_t sk_num;
    struct sockaddr_in6 remote;
    uint32_t len;
} Connection;

// for the TCP server side
int tcpServerSetup(int serverPort);
int tcpAccept(int mainServerSocket, int debugFlag);

// for the TCP client side
int tcpClientSetup(char * serverName, char * serverPort, int debugFlag);

// For UDP Server and Client
int udpServerSetup(int serverPort);
int setupUdpClientToServer(struct sockaddr_in6 *serverAddress, char * hostName, int serverPort);

int udpServerSetup(int portNumber);
int udpClientSetup(char *hostName, int portNumber, Connection *connection);
int safeGetUdpSocket();
void printIPv6Info(struct sockaddr_in6 *client);

#endif
