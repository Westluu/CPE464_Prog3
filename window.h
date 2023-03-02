#ifndef WINDOW_H
#define WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "packet.h"

#define MAXBUF 1407

typedef struct windowBuf
{
    uint8_t packet[MAXBUF];
    uint8_t valid;
} WindowBuf;

typedef struct window
{
    uint32_t lower;
    uint32_t current;
    uint32_t upper;
    uint32_t size;
    WindowBuf *buf;
} Window;

void init_window(Window *window, int size);

#endif
