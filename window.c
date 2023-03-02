#include "window.h"

void init_window(Window *window, int size) {
    window->lower = 1;
    window->current = 1;
    window->upper = size;
    window->size = size;
    WindowBuf *buf = malloc(sizeof(WindowBuf) * size);
    window->buf = buf;
}

void init_buffer(Window *window, uint32_t size) {
    int i = 0;
    WindowBuf start;
    start.valid = 0;
    for (i = 0; i < size; i++) {
        window->buf[i] = start;
    }
    return window;
}

int check_closed(Window *window) {
    if (window->current >= window->upper) {
        return 1;
    }
    return 0;
}

void insert_packet(Window *window, uint8_t packet[]) {
    uint32_t i = 0;
    uint32_t seq_num = ntohl(get_seqnum(packet));
    WindowBuf window_pkt = window->buf[i];
}