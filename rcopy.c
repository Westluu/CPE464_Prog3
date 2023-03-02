#include "packet.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "cpe464.h"
#include "window.h"

typedef enum State STATE;
enum State{
    START, FILENAME, SEND_DATA, WAIT, CHECK_FLAGS, DONE
};

void processFile(char *argv[]);
STATE send_filename(char *filename, Connection *server, uint32_t seq_num, uint32_t window_size, uint32_t buffer_size);
STATE connect_server(char *argv[], Connection *server);
STATE send_data(FILE *upload_file, Connection *server, uint32_t seq_num, Window window);
int checkArgs(int argc, char * argv[]);

int count = 0;
int main(int argc, char *argv[]) {
    setupPollSet();
    sendtoErr_init(atof(argv[5]), DROP_OFF, FLIP_OFF, DEBUG_ON, RSEED_OFF);
    processFile(argv);
}

void processFile(char *argv[]) {
    Window window;
    uint32_t seq_num = 0;
    STATE state = START;

    check_file();
    init_window(&window, atoi(argv[5]));
    Connection *server = (Connection *) calloc(1, sizeof(Connection));

    while (state != DONE) {
        if (count > 9) {
            state = DONE;
        }
        switch (state) {
            case START:
                state = connect_server(argv, server);
                break;

            case FILENAME:
                seq_num = 0;
                state = send_filename(argv[2], server, seq_num, atoi(argv[3]), atoi(argv[4]));
                seq_num++;
                break;
    
            case SEND_DATA:
                state = send_data(argv[1], server, seq_num);
                break;

            case WAIT:
                break;

            case CHECK_FLAGS:
                break;

            case DONE:
                printf("DONE\n");
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
        uint32_t recv_len = CsafeRecvfrom(socket, recv_pkt, MAXBUF, server);
        flag = get_flag(recv_pkt);
        
        if (flag == OK_FILENAME) {
            printf("OK Filename");
            return SEND_DATA;
        } 

        else if (flag == BAD_FILENAME) {
            printf("Bad Filename\n");
            return DONE;
        }

    } else { //Did not recieve file ack
        count++;
    }

    return DONE;
}

STATE send_data(FILE *upload_file, Connection *server, uint32_t seq_num, Window window) {
    return DONE;
}

int checkArgs(int argc, char * argv[]) {
    int portNumber = 0;
	
	if (argc != 8)
	{
		printf("usage: %s from-filename to-filename window-size buffer-size error-percent remote-machine remote-port\n", argv[0]);
		exit(1);
	}

	portNumber = atoi(argv[7]);
	return portNumber;
}