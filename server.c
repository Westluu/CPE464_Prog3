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
int processArgs(int argc, char *argv[]);
void handleZombie(int sig);
int count = 0;

STATE filename(Connection *client, uint8_t recv_buf[], uint32_t recv_len, uint32_t window_size, uint32_t buffer_size, uint8_t flag, FILE *file);

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
    // Window window;
    STATE state = FILENAME;
    FILE *file = 0;
    uint32_t seq_num = 0;
    uint32_t window_size = get_windowSize(recv_buf);
    uint32_t buffer_size = get_bufferSize(recv_buf);
    uint8_t flag = get_flag(recv_buf);

    while (state != DONE) {
        if (count < 9) {

        }
        switch (state) {
            case FILENAME:
                state = filename(client, recv_buf, recv_len, window_size, buffer_size, flag, file);
                break;
            
            case RECV_DATA:
                break;
            
            case DONE:
                break;
            
            default:
                printf("Error -in default state\n");
                break;
        }
    }
}

STATE filename(Connection *client, uint8_t recv_buf[], uint32_t recv_len, uint32_t window_size, uint32_t buffer_size, uint8_t flag, FILE *file) {
    char filename[MAXFILE];
    uint8_t ack[MAXBUF];
    uint32_t ack_len = 0;
    // client->remote.sin6_family = AF_INET6;
    // close(client->sk_num);
    client->sk_num = safeGetUdpSocket();

    addToPollSet(client->sk_num);

    get_filename(recv_buf, filename);
    file = fopen(filename, "r");

    if (file == NULL) {
        printf("BAD\n");
        //send bad file 
        ack_len = create_header(ack, 0, BAD_FILENAME);
        printf("clinet socket: %d\n", client->sk_num);
        safeErrSend(ack, ack_len, client);
        printf("sent bad file\n");
        return RECV_DATA;

    } else {
        printf("OK\n");
        //send Ok file
        ack_len = create_header(ack, 0, OK_FILENAME);
        safeErrSend(ack, ack_len, client);
        printf("send ok file\n");
        return DONE;
    }
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

void handleZombie(int sig) {
    int stat = 0;
    while (waitpid(-1, &stat, WNOHANG) > 0) {}
}