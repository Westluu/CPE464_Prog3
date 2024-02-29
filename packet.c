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

    return pdu_len; 
}

void send_seq_pkt(Window *window, uint32_t seq_num, Connection *server) {
    uint8_t seq_pkt[MAXBUF];
    uint32_t seq_len = read_packet(window, seq_pkt, seq_num);
    safeErrSend(seq_pkt, seq_len, server);
}


void Ssend_SREJ_pkt(uint32_t seq_num, uint32_t SREJ_num, Connection *client) {
    uint8_t SREJ_pkt[MAXBUF];
    uint32_t SREJ_len = create_RR_packet(SREJ_pkt, seq_num, SREJ, SREJ_num);
    safeErrSend(SREJ_pkt, SREJ_len, client);
}


void Csend_SREJ_pkt(Window *window, uint8_t recv_pkt[], Connection *server) {
    uint8_t SREJ_pkt[MAXBUF];
    uint32_t SREJ_num = get_SREJ(recv_pkt);
    uint32_t SREJ_len = read_packet(window, SREJ_pkt, SREJ_num);
    safeErrSend(SREJ_pkt, SREJ_len, server);
}


void send_RR_packet(uint32_t seq_num, uint32_t RR_num, Connection *client) {
    uint8_t RR_buff[MAXBUF];
    uint32_t RR_len = create_RR_packet(RR_buff, seq_num, RR, RR_num);
    safeErrSend(RR_buff, RR_len, client);
}

int create_RR_packet(uint8_t RR_buff[], uint32_t seq_num, uint8_t flag, uint32_t RR_num) {
    uint32_t offset = 0;
    uint16_t checksum = 0;
    uint32_t nseq_num = htonl(seq_num);
    uint32_t nRR_num = htonl(RR_num);

    int pdu_len = 0;
    memcpy(RR_buff, &nseq_num, SEQ_LEN);
    offset += SEQ_LEN;

    memcpy(RR_buff + offset, &checksum, CHK_LEN);
    offset += CHK_LEN;
    memcpy(RR_buff + offset, &flag, FLAG_LEN);
    offset += FLAG_LEN;
    memcpy(RR_buff + offset, &nRR_num, RR_LEN);
    pdu_len = offset + RR_LEN;

    checksum = in_cksum((unsigned short *) RR_buff, pdu_len);
    memcpy(RR_buff + SEQ_LEN, &checksum, CHK_LEN);

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

    return pdu_len;
}

uint32_t create_file_data(uint8_t filename_buf[], char *filename, uint32_t window_size, uint32_t buffer_size) {
    uint32_t offset = 0;
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

    return offset;
}

void send_EOF_pkt(uint8_t seq_num, Connection *server) {
    uint8_t EOF_buff[MAXBUF];
    uint32_t EOF_len = create_header(EOF_buff, seq_num, EOF_FLAG);
    safeErrSend(EOF_buff, EOF_len, server);
}


uint32_t get_SREJ(uint8_t recv_packet[]) {
    uint32_t SREJ_num = 0;
    memcpy(&SREJ_num, recv_packet + HEADER_LEN, SREJ_LEN);
    return ntohl(SREJ_num);
}



uint32_t get_RR(uint8_t recv_packet[]) {
    uint32_t RR_num = 0;
    memcpy(&RR_num, recv_packet + HEADER_LEN, RR_LEN);
    return ntohl(RR_num);
}

uint8_t get_flag(uint8_t recv_packet[]) {
    return recv_packet[SEQ_LEN + CHK_LEN];
}

uint32_t get_seqnum(uint8_t recv_packet[]) {
    uint32_t seq_num = 0;
    memcpy(&seq_num, recv_packet, SEQ_LEN);
    return ntohl(seq_num);
}

uint32_t get_windowSize(uint8_t recv_packet[]) {
    uint32_t window_size = 0;
    memcpy(&window_size, recv_packet + HEADER_LEN, WINDOW_LEN);
    return ntohl(window_size);
}

uint32_t get_bufferSize(uint8_t recv_packet[]) {
    uint32_t buffer_size = 0;
    memcpy(&buffer_size, recv_packet + HEADER_LEN + WINDOW_LEN, BUFFER_LEN);
    return ntohl(buffer_size);
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