#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "window.h"
#include "networks.h"
#include "safeUtil.h"
#include "checksum.h"

#define MAXBUF 1407
#define MAXFILE 100
#define SEQ_LEN 4
#define RR_LEN 4
#define SREJ_LEN 4
#define CHK_LEN 2
#define FLAG_LEN 1
#define HEADER_LEN 7

#define WINDOW_LEN 4
#define BUFFER_LEN 4
#define FILE_LEN 1

#define DATA_PACKET 3
#define RR 5
#define SREJ 6 
#define SEND_FILE_FLAG 7
#define OK_FILENAME 8
#define BAD_FILENAME 9
#define EOF_FLAG 10

void print_hex(uint8_t buff[], int len);

int create_packet(uint8_t buff[], uint32_t seq_num, uint8_t flag, uint8_t data[], uint32_t data_len);
int create_header(uint8_t buff[], uint32_t seq_num, uint8_t flag);
uint32_t create_file_data(uint8_t filename_buf[], char *filename, uint32_t window_size, uint32_t buffer_size);

void send_EOF_pkt(uint8_t seq_num, Connection *server);
void send_seq_pkt(Window *window, uint32_t seq_num, Connection *server);
void Csend_SREJ_pkt(Window *window, uint8_t recv_pkt[], Connection *server);
void Ssend_SREJ_pkt(Window *window, uint32_t SREJ_num, Connection *client);

void send_RR_packet(uint32_t seq_num, uint32_t RR_num, Connection *client);
int create_RR_packet(uint8_t RR_buff[], uint32_t seq_num, uint8_t flag, uint32_t RR_num);

uint32_t get_SREJ(uint8_t recv_packet[]);
uint32_t get_RR(uint8_t recv_packet[]);

uint8_t get_flag(uint8_t recv_packet[]);
uint32_t get_seqnum(uint8_t recv_packet[]);
uint32_t get_windowSize(uint8_t recv_packet[]);
uint32_t get_bufferSize(uint8_t recv_packet[]);
char *get_filename(uint8_t recv_packet[], char *filename);

#endif
