#include "window.h"

void init_window(Window *window, int size) {
    window->lower = 1;
    window->current = 1;
    window->upper = size;
    window->size = size;
    WindowBuf *buf = malloc(sizeof(WindowBuf) * size);
    window->buf = buf;
}

void check_closed(Window *window) {
    
}

void add_packet(Window *window, uint8_t packet) {
    
}