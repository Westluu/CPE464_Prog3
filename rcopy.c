#include "packet.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "cpe464.h"
#include "window.h"

typedef enum State STATE;
enum State{
    START, FILENAME, SEND_DATA, CHECK_FLAG, WAIT, DONE
};

void processFile(char *argv[]);
STATE send_filename(char *filename, Connection *server, uint32_t seq_num, uint32_t window_size, uint32_t buffer_size);
STATE connect_server(char *argv[], Connection *server);
STATE send_data(FILE *upload_file, Connection *server, Window *window, uint32_t buffer_size);
void check_flag(Window *window, Connection *server, uint8_t flag, uint8_t recv_pkt[], uint32_t recv_len);
FILE *check_file(char *file);
int checkArgs(int argc, char * argv[]);

int count = 0;
int read_EOF = 0;
uint32_t rr_num = 0;

int main(int argc, char *argv[]) {
    setupPollSet();
    sendtoErr_init(atof(argv[5]), DROP_OFF, FLIP_OFF, DEBUG_ON, RSEED_OFF);
    processFile(argv);
    return 1;
}

void processFile(char *argv[]) {
    Window window;
    STATE state = START;
    FILE *upload_file = check_file(argv[1]);

    init_window(&window, atoi(argv[3]));
    Connection *server = (Connection *) calloc(1, sizeof(Connection));
    while (state != DONE) {
        if (count > 9) {
            printf("COUNT REACHED 10");
            state = DONE;
        }
        switch (state) {

            case START:
                state = connect_server(argv, server);
                break;

            case FILENAME:
                state = send_filename(argv[2], server, get_current(&window), atoi(argv[3]), atoi(argv[4]));
                break;
    
            case SEND_DATA:
                state = send_data(upload_file, server, &window, atoi(argv[4]));
                break;

            case DONE:
                printf("DONE\n");
                exit(0);
                break;

            default:
                printf("Error -in default state\n");
                break;
        }
    }
}

STATE connect_server(char *argv[], Connection *server){
    if (server->sk_num > 0) {
        close(server->sk_num);
    }

    if (udpClientSetup(argv[6], atoi(argv[7]), server) < 0) {
        //close socket and restart
        return DONE;

    } else {
        printf("Connected to server\n");
        addToPollSet(server->sk_num);
        return FILENAME;
    }
}

STATE send_filename(char *filename, Connection *server, uint32_t seq_num, uint32_t window_size, uint32_t buffer_size) {
    int socket;

    //send out the filename
    uint8_t packet[MAXBUF];
    uint8_t data[MAXBUF];
    uint32_t data_len = create_file_data(data, filename, window_size, buffer_size);
    uint32_t pkt_len = create_packet(packet, seq_num, SEND_FILE_FLAG, data, data_len);

    safeErrSend(packet, pkt_len, server);

    //Wait 1 second for FileName ACK
    if ((socket = pollCall(1000)) >= 0){  //recieved the file ACK
        uint8_t recv_pkt[MAXBUF];
        uint8_t flag = 0;
        CsafeRecvfrom(socket, recv_pkt, MAXBUF, server);
        flag = get_flag(recv_pkt);

        if (flag == OK_FILENAME) {
            printf("OK Filename\n");
            return SEND_DATA;
        } 
        
        else if (flag == BAD_FILENAME) {
            printf("Bad Filename\n");
            return DONE;
        }

    } else { //Did not recieve file ack
        count++;
        return START; //look at papaer
    }
    return FILENAME;
}

//hang 1 second resend data
STATE send_data(FILE *upload_file, Connection *server, Window *window, uint32_t buffer_size) {
    // printf("in send data\n");
    uint32_t read_len = 0;
    uint8_t read_buf[buffer_size];
    int socket = 0;

    //Open Window
    print_window(window);

    if (!check_closed(window) && !read_EOF) {
        printf("Window Open\n");
        printf("read EOF: %d\n", read_EOF);
        printf("last: %d\n", check_last(window));
        read_len = fread(read_buf, sizeof(uint8_t), buffer_size, upload_file); //safe read
        read_buf[buffer_size] = '\0';
        
        if (read_len > 0) {
            uint8_t data_buff[MAXBUF];
            uint32_t data_len = create_packet(data_buff, get_current(window), DATA_PACKET, read_buf, read_len);
            safeErrSend(data_buff, data_len, server);
            insert_packet(window, data_buff, data_len);
            update_current(window);
            count = 0;
        } else {
            read_EOF = 1;
        }

        while ((socket = pollCall(0)) >= 0) {
            printf("in check flags\n");
            uint8_t recv_pkt[MAXBUF];
            CsafeRecvfrom(socket, recv_pkt, MAXBUF, server);
            uint8_t flag = get_flag(recv_pkt);
            check_flag(window, server, flag, recv_pkt, read_len);
        }

    } 
    
    //Full Window
    else if (check_full(window) || (read_EOF && !check_last(window)) ) {
        printf("window full\n");
        if ((socket = pollCall(1000)) < 0) {
            printf("Sent lowest SEQ:%d pkt\n", rr_num);
            send_seq_pkt(window, rr_num, server);
            printf("count: %d\n", count);
            count++;
        }
        else {
            uint8_t recv_pkt[MAXBUF];
            CsafeRecvfrom(socket, recv_pkt, MAXBUF, server);
            uint8_t flag = get_flag(recv_pkt);
            check_flag(window, server, flag, recv_pkt, read_len);
         }
    }

    //handle EOF
    else if (check_last(window) && read_EOF) {
        send_EOF_pkt(window->current, server);
        return DONE;
    }

    return SEND_DATA;
}


void check_read(int read_len) {
    if (read_len == 0){
        read_EOF = 1;
    } else if (read_len) {
        perror("fread error\n");
        exit(0);
    }
}

void check_flag(Window *window, Connection *server, uint8_t flag, uint8_t recv_pkt[], uint32_t recv_len) {
     if (flag == RR) {
        printf("got RR");
        rr_num = get_RR(recv_pkt);
        printf("slide window\n");
        slide_window(window, rr_num);

    } else if (flag == SREJ) {
        printf("got SREJ");
        Csend_SREJ_pkt(window, recv_pkt, server);
        count = 0;
    }
}

FILE *check_file(char *file) {
    FILE *f;
    if ( (f = fopen(file, "r")) == NULL) {
        printf("To-File Invlaid, exiting");
        exit(1);
    }
    return f;
}

int checkArgs(int argc, char * argv[]) {
    int portNumber = 0;
	
	if (argc != 8) {
		printf("usage: %s from-filename to-filename window-size buffer-size error-percent remote-machine remote-port\n", argv[0]);
		exit(1);
	}
	portNumber = atoi(argv[7]);
	return portNumber;
}