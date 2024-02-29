#include "window.h"

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

void init_window(Window *window, int size) {
    window->lower = 0;
    window->current = 0;
    window->upper = size;
    window->size = size;
    printf("size: %d\n",size);
    WindowBuf *buf = safe_malloc(sizeof(WindowBuf) * size);
    window->buf = buf;
    init_buffer(window, size);
}

void init_buffer(Window *window, uint32_t size) {
    int i = 0;
    for (i = 0; i < size; i++) {
        (window->buf)[i].valid = 0;
        (window->buf)[i].pkt_len = 0;
    }
}

void update_current(Window *window) {
    window->current += 1;
}

int check_last(Window *window) {
    if (window->lower == window->current) {
        return 1;
    }
    return 0;
}

int check_full(Window *window) {
    if (window->current == window->upper) {
        return 1;
    }
    return 0;
}

int check_closed(Window *window) {
    if (window->current >= window->upper) {
        return 1;
    }
    return 0;
}

void slide_window(Window *window, int new_lower) {
    int j;
    int i;
    window->upper = new_lower + window->size;
    window->lower = new_lower;
}

void remove_packet(Window *window, uint32_t seq_num) {
    int i = seq_num % window->size;
    (window->buf)[i].valid = 0;
}

uint32_t get_current(Window *window) {
    return window->current;
}

uint32_t get_lower(Window *window) {
    return window->lower;
}

uint8_t get_valid(Window *window, uint32_t seq_num) {
    int i = seq_num % window->size;
    return (window->buf)[i].valid;
}

void set_valid(Window *window, uint32_t seq_num, uint8_t valid) {
    int i = seq_num % window->size;
    (window->buf)[i].valid = valid;
}

uint32_t read_packet(Window *window, uint8_t buffer[], uint32_t seq_num) {
    int i = seq_num % window->size;
    uint32_t buff_len = (window->buf)[i].pkt_len;
    memcpy(buffer, (window->buf)[i].packet, buff_len);
    return buff_len;
}

uint8_t *get_packet(Window *window, uint32_t seq_num) {
    int i = seq_num % window->size;
    return (window->buf)[i].packet;
}

void insert_packet(Window *window, uint8_t packet[], uint32_t pkt_len) {
    int i = 0;
    uint32_t seq_num = ntohl(wget_seqnum(packet));
    i = seq_num % window->size;
    (window->buf)[i].pkt_len = pkt_len;
    (window->buf)[i].valid = 1;
    memcpy((window->buf)[i].packet, packet, pkt_len);
}

uint32_t wget_seqnum(uint8_t recv_packet[]) {
    uint32_t seq_num = 0;
    memcpy(&seq_num, recv_packet, SEQ_LEN);
    return seq_num;
}

void print_window(Window *window) {
    int i;
    printf("Window l:%d, c:%d, u:%d\n", window->lower, window->current, window->upper);
    for (i = 0; i < window->size; i++) {
        printf("i:%d , v:%d\n", i, (window->buf)[i].valid);
    }
}