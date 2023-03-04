#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "pollLib.h"
#include "cpe464.h"
#include "networks.h"
#include "packet.h"
#include "window.h"

typedef enum State STATE;
enum State{
    START, FILENAME, RECV_DATA, DONE
};

void process_server(int serverScoketNumber);
void process_client(uint32_t serverSocketNumber, uint8_t recv_buf[], uint32_t recv_len, Connection *client);
void loop_window(Window *window, uint32_t recv_len, FILE *file, Connection *client);
void recover_packet(Window *window, uint32_t recv_seq, Connection *client);
int processArgs(int argc, char *argv[]);
void open_file(uint8_t recv_buf[]);
uint32_t max(uint32_t x, uint32_t y);
void handleZombie(int sig);
int count = 0;
uint32_t highest_seq = 0;
uint32_t seq_num = 0;
FILE *file = 0;

STATE filename(Connection *client, uint8_t recv_buf[], uint32_t recv_len, uint32_t window_size, uint32_t buffer_size, uint8_t flag, FILE *file);
STATE recv_data(Connection *client, FILE *file, Window *window);

int main(int argc, char *argv[]) {
    int serverSocketNumber = 0;
    int portNumber = 0;

    setupPollSet();
    portNumber = processArgs(argc, argv);
    sendtoErr_init(atof(argv[1]), DROP_OFF, FLIP_OFF, DEBUG_ON, RSEED_OFF);
    serverSocketNumber = udpServerSetup(portNumber);
    process_server(serverSocketNumber);
    return 0;
}

void process_server(int serverSocketNumber) {
    pid_t pid = 0;
    Connection *client = (Connection *) calloc(1, sizeof(Connection));
    uint8_t recv_buf[MAXBUF];
    uint32_t recv_len = 0;

    signal(SIGCHLD, handleZombie);

    while (1) {
        recv_len = CsafeRecvfrom(serverSocketNumber, recv_buf, MAXBUF, client);

        if (recv_len != CRC_ERROR) {
            if ((pid = fork()) < 0 ) {
                perror("fork");
                exit(-1);
            }
        }

        if (pid == 0) {
            //in child process
            printf("Child for() - child pid: %d\n", getpid());
            process_client(serverSocketNumber, recv_buf, recv_len, client);
            exit(0);
        }
    }
}

void process_client(uint32_t serverSocketNumber, uint8_t recv_buf[], uint32_t recv_len, Connection *client) {
    Window window;
    STATE state = FILENAME;

    uint32_t window_size = get_windowSize(recv_buf);
    uint32_t buffer_size = get_bufferSize(recv_buf);
    init_window(&window, window_size);
    uint8_t flag = get_flag(recv_buf);
    open_file(recv_buf);

    while (state != DONE) {
        if (count > 9) {
            printf("COUNT REACHED 10");
            state = DONE;
        }

        switch (state) {
            case FILENAME:
                state = filename(client, recv_buf, recv_len, window_size, buffer_size, flag, file);
                break;
            
            case RECV_DATA:
                state = recv_data(client, file, &window);
                break;
            
            case DONE:
                exit(0);
                break;
            
            default:
                printf("Error -in default state\n");
                break;
        }
    }
}

STATE filename(Connection *client, uint8_t recv_buf[], uint32_t recv_len, uint32_t window_size, uint32_t buffer_size, uint8_t flag, FILE *file) {
    uint8_t ack[MAXBUF];
    uint32_t ack_len = 0;

    client->sk_num = safeGetUdpSocket();
    addToPollSet(client->sk_num);

    if (file == NULL) {
        //send bad file 
        ack_len = create_header(ack, 0, BAD_FILENAME);
        safeErrSend(ack, ack_len, client);
        printf("sent bad file\n");
        return DONE;
    } else {
        //send Ok file
        ack_len = create_header(ack, 0, OK_FILENAME);
        safeErrSend(ack, ack_len, client);
        printf("send ok file\n");
        return RECV_DATA;
    }
}

STATE recv_data(Connection *client, FILE *file, Window *window) {
    int socket = 0;
    if ((socket = pollCall(10000)) >= 0) {
        printf("in here\n");
        uint8_t recv_pkt[MAXBUF];
        uint32_t recv_len = CsafeRecvfrom(socket, recv_pkt, MAXBUF, client);

        uint8_t flag = get_flag(recv_pkt);
        if (flag == EOF_FLAG) {
            return DONE;
        }
        
        //insert packet into the window
        insert_packet(window, recv_pkt, recv_len);

        uint32_t recv_seq_num = get_seqnum(recv_pkt);
        int checksum = in_cksum((unsigned short *) recv_pkt, recv_len);

        //If we get expected Data Packet
        if (recv_seq_num == seq_num && checksum == 0) {
            printf("Valid Data pkt\n");
            loop_window(window, recv_len, file, client);
            send_RR_packet(seq_num - 1 , seq_num, client);
            return RECV_DATA;
        }

        //If we get a duplicate Packet
        else if (recv_seq_num < seq_num && checksum == 0) {
            printf("Got Duplicate Packet\n");
            send_RR_packet(seq_num - 1, seq_num, client);
        }

        //If we get a out of order packet
        else if (recv_seq_num > seq_num && checksum == 0) {
            printf("higher eexpect packet\n");
            printf("recv_seq_num: %d\n", recv_seq_num);
            recover_packet(window, recv_seq_num, client);
            return RECV_DATA;
        }

        //Corrupt Packet
        else if (checksum != 0) {
            return RECV_DATA;
        }

    }
    return DONE;
}

void recover_packet(Window *window, uint32_t recv_seq, Connection *client) {
    printf("here\n");
    int i = 0;
    highest_seq = max(highest_seq, seq_num);
    for (i = highest_seq; i < recv_seq; i++) {
        printf("recv_seq_num: %d\n", recv_seq);
        Ssend_SREJ_pkt(window, seq_num, client);
    }
    highest_seq = max(highest_seq, recv_seq +1);
}



void loop_window(Window *window, uint32_t recv_len, FILE *file, Connection *client) {
    while ((window->buf)[seq_num].valid) {
        uint32_t data_len = recv_len - HEADER_LEN;
        fwrite((window->buf)[seq_num].packet + HEADER_LEN, sizeof(uint8_t), data_len, file);
        fflush(file);
        printf("wrote to file");
        (window->buf)[seq_num].valid = 0;
        seq_num++;
    }
}


void open_file(uint8_t recv_buf[]) {
    char file_name[MAXFILE];
    get_filename(recv_buf, file_name);
    file = fopen(file_name, "w");
}

int processArgs(int argc, char *argv[]) {
    int portNumber = 0;
    if (argc < 2 || argc > 3) {
        printf("Usage %s error_rate [port number]\n", argv[0]);
        exit(-1);
    }
    if (argc == 3) {
        portNumber = atoi(argv[2]);
    } else {
        portNumber = 0;
    }
    return portNumber;
}

uint32_t max(uint32_t x, uint32_t y) {
    if (x > y) {
        return x;
    }
    return y;
}



void handleZombie(int sig) {
    int stat = 0;
    while (waitpid(-1, &stat, WNOHANG) > 0) {}
}