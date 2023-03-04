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


#define MAXBUF 1407

typedef struct WindowBuf
{
    uint8_t packet[MAXBUF];
    uint32_t pkt_len;
    uint8_t valid;
} WindowBuf;

typedef struct Window {
    uint32_t lower;
    uint32_t current;
    uint32_t upper;
    uint32_t size;
    WindowBuf *buf;
} Window;

void cprint_hex(uint8_t buff[], int len);
void init_window(Window *window, int size);
void init_buffer(Window *window, uint32_t size);
uint32_t get_current(Window *window);

int check_full(Window *window);
int check_closed(Window *window);
int check_last(Window *window);
void update_current(Window *window);

void slide_window(Window *window, int new_lower);
void remove_packet(Window *window, uint32_t seq_num);
uint32_t read_packet(Window *window, uint8_t buffer[], uint32_t seq_num);
void insert_packet(Window *window, uint8_t packet[], uint32_t pkt_len);
uint32_t wget_seqnum(uint8_t recv_packet[]);
void print_window(Window *window);

#endif
