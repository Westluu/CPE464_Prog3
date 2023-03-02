#include "packet.h"

void print_hex(uint8_t buff[], int len) {
    int i = 0;
    for (i=0; i < len; i++) {
        printf("%02x ", buff[i]);
        fflush(stdout);
    }
    printf("\n");
}

int create_header(uint8_t buff[], uint32_t seq_num, uint8_t flag) {
    uint32_t offset = 0;
    uint16_t checksum = 0;
    uint32_t nseq_num = htonl(seq_num);
    int pdu_len = 0;

    memcpy(buff, &nseq_num, SEQ_LEN);
    offset += SEQ_LEN;

    memcpy(buff + offset, &checksum, CHK_LEN);
    offset += CHK_LEN;

    memcpy(buff + offset, &flag, FLAG_LEN);
    offset += FLAG_LEN;

    pdu_len = offset;
    checksum = in_cksum((unsigned short *) buff, pdu_len);
    memcpy(buff + SEQ_LEN, &checksum, CHK_LEN);
    
    // printf("HEADER\n");
    // printf("Len: %d\n", pdu_len);

    // print_hex(buff, pdu_len);

    // if ( in_cksum((unsigned short *) buff, pdu_len) == 0) {
    //     printf("WORKED\n");
    // } else {
    //     printf("FAILED\n");
    // }
    return pdu_len; 
}

int create_packet(uint8_t buff[], uint32_t seq_num, uint8_t flag, uint8_t data[], uint32_t data_len) {
    uint32_t offset = 0;
    uint16_t checksum = 0;
    uint32_t nseq_num = htonl(seq_num);
    int pdu_len = 0;
    memcpy(buff, &nseq_num, SEQ_LEN);
    offset += SEQ_LEN;

    memcpy(buff + offset, &checksum, CHK_LEN);
    offset += CHK_LEN;
    memcpy(buff + offset, &flag, FLAG_LEN);
    offset += FLAG_LEN;
    memcpy(buff + offset, data, data_len);
    pdu_len = offset + data_len;

    checksum = in_cksum((unsigned short *) buff, pdu_len);
    memcpy(buff + SEQ_LEN, &checksum, CHK_LEN);

    // printf("FULL\n");
    // printf("Len: %d\n", pdu_len);

    // print_hex(buff, pdu_len);

    // if ( in_cksum((unsigned short *) buff, pdu_len) == 0) {
    //     printf("WORKED\n");
    // } else {
    //     printf("FAILED");
    // }
    return pdu_len;
}

uint32_t create_file_data(uint8_t filename_buf[], char *filename, uint32_t window_size, uint32_t buffer_size) {
    uint32_t offset = 0;

    printf("filename: %s\n", filename);
    uint8_t name_len = strlen(filename);
    uint32_t nwindow_size = htonl(window_size);
    uint32_t nbuffer_size = htonl(buffer_size);

    memcpy(filename_buf, &nwindow_size, 4);
    offset += 4;
    memcpy(filename_buf + offset, &nbuffer_size, 4);
    offset += 4;
    memcpy(filename_buf + offset, &name_len, 1);
    offset += 1;
    memcpy(filename_buf + offset, filename, name_len);
    offset += name_len;

    // printf("DATA\n");
    // printf("Len: %d\n", offset);
    // print_hex(filename_buf, offset);
    return offset;
}

uint8_t get_flag(uint8_t recv_packet[]) {
    return recv_packet[SEQ_LEN + CHK_LEN];
}

uint32_t get_seqnum(uint8_t recv_packet[]) {
    uint32_t seq_num = 0;
    memcpy(&seq_num, recv_packet, SEQ_LEN);
    return seq_num;
}

uint32_t get_windowSize(uint8_t recv_packet[]) {
    uint32_t window_size = 0;
    memcpy(&window_size, recv_packet + HEADER_LEN, WINDOW_LEN);
    return window_size;
}

uint32_t get_bufferSize(uint8_t recv_packet[]) {
    uint32_t buffer_size = 0;
    memcpy(&buffer_size, recv_packet + HEADER_LEN + WINDOW_LEN, BUFFER_LEN);
    return buffer_size;
}

uint8_t get_fileLen(uint8_t recv_packet[]) {
    uint8_t file_len = 0;
    memcpy(&file_len, recv_packet + HEADER_LEN + WINDOW_LEN + BUFFER_LEN, FILE_LEN);
    return file_len;
}

char *get_filename(uint8_t recv_packet[], char *filename) {
    uint8_t file_len = get_fileLen(recv_packet);
    memcpy(filename, recv_packet + HEADER_LEN + WINDOW_LEN + BUFFER_LEN + FILE_LEN, file_len);
    filename[file_len] = '\0';
    printf("File: %s\n", filename);
    return filename;    
}