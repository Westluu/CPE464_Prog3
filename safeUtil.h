// 
// Writen by Hugh Smith, April 2020
//
// Put in system calls with error checking.

#ifndef __SAFEUTIL_H__
#define __SAFEUTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "networks.h"

struct sockaddr;

int safeRecvfrom(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int * addrLen);
int CsafeRecvfrom(int socketNum, uint8_t *buf, int len, Connection *from);
int safeSendto(int socketNum, void * buf, int len, int flags, struct sockaddr *srcAddr, int addrLen);

int safeRecv(int socketNum, void * buf, int len, int flags);
int safeSend(int socketNum, void * buf, int len, int flags);

int safeErrSend(uint8_t *data, uint32_t len, Connection *to);

void * srealloc(void *ptr, size_t size);
void * sCalloc(size_t nmemb, size_t size);
void *safe_malloc(size_t size);


#endif
