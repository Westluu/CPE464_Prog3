
// 
// Writen by Hugh Smith, April 2020, Feb. 2021
//
// Put in system calls with error checking
// keep the function paramaters same as system call

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cpe464.h"
#include "safeUtil.h"
#include "networks.h"

int safeRecvfrom(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int * addrLen)
{
	int returnValue = 0;
	if ((returnValue = recvfrom(socketNum, buf, (size_t) len, flags, srcAddr, (socklen_t *) addrLen)) < 0)
	{
		perror("recvfrom: ");
		exit(-1);
	}
	
	return returnValue;
}

int CsafeRecvfrom(int socketNum, uint8_t *buf, int len, Connection *from)
{
	int recv_len = 0;
	from->len = sizeof(struct sockaddr_in6);

	if ((recv_len = recvfrom(socketNum, buf, (size_t) len, 0, (struct sockaddr *) &(from->remote), &from->len)) < 0) {
		perror("recvfrom: ");
		exit(-1);
	}
	
	return recv_len;
}

int safeSendto(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int addrLen)
{
	int returnValue = 0;
	if ((returnValue = sendto(socketNum, buf, (size_t) len, flags, srcAddr, (socklen_t) addrLen)) < 0)
	{
		perror("sendto: ");
		exit(-1);
	}
	
	return returnValue;
}

int safeErrSend(uint8_t *data, uint32_t len, Connection *to)
{
	int send_len = 0;

	if ((send_len = sendtoErr(to->sk_num, data, len, 0, (struct sockaddr *) &(to->remote), sizeof(struct sockaddr_in6))) < 0)
	{
		perror("send Err: ");
		exit(-1);
	}
	
	return send_len;
}

int safeRecv(int socketNum, void * buf, int len, int flags)
{
	int returnValue = 0;
	if ((returnValue = recv(socketNum, buf, (size_t) len, flags)) < 0)
	{
		perror("recv: ");
		exit(-1);
	}
	
	return returnValue;
}

int safeSend(int socketNum, void * buf, int len, int flags)
{
	int returnValue = 0;
	if ((returnValue = send(socketNum, buf, (size_t) len, flags)) < 0)
	{
		perror("send: ");
		exit(-1);
	}
	
	return returnValue;
}

void * srealloc(void *ptr, size_t size)
{
	void * returnValue = NULL;
	
	if ((returnValue = realloc(ptr, size)) == NULL)
	{
		printf("Error on realloc (tried for size: %d\n", (int) size);
		exit(-1);
	}
	
	return returnValue;
} 

void * sCalloc(size_t nmemb, size_t size)
{
	void * returnValue = NULL;
	if ((returnValue = calloc(nmemb, size)) == NULL)
	{
		perror("calloc");
		exit(-1);
	}
	return returnValue;
}

void *safe_malloc(size_t size) {
    void *a = malloc(size);
    if (a == NULL) {
        perror("malloc");
        exit(-1);
    }
    return a;
}
