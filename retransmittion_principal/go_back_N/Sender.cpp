#include "../Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512
#define TIMEOUT 5
int WINDOWSIZE = 4;

int send_base = 0;
int nextseqnum = 0;

pthread_mutex_t mutex;

typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in serveraddr;
    char buf[BUFSIZE + 1];
} socket_sockaddrin;


const char* packets[6] = {"packet 0", "packet 1", "packet 2", "packet 3", "packet 4", "packet 5"};
int timeout[512] = {0,};
int error_2 = 1;
void* send_packet(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in serveraddr = socket->serveraddr;
    socklen_t addrlen;
    int packet_num = socket->buf[7] -'0';

    if(socket->buf[7] != '2' || error_2 !=1 ){
       timeout[packet_num] = 0;
       send(sock, socket->buf, BUFSIZE, 0);
    }
    else{

        for(int i=0;i<5;i++){
            sleep(1);

            timeout[packet_num]++;
       }
        error_2 = 0;
    }
    pthread_exit(NULL);
}

void *receive(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in serveraddr = socket->serveraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    char rcvnum = 0;
    int ignore = 0;
    while(1){


        int retval;

        retval = recv(sock, buf, BUFSIZE, 0);

        if(retval != 0){
            if(ignore == 0){
             if(buf[4] == rcvnum){
                 printf("\"%s\" is received and ignored.\n", buf);
                 ignore = 1;

             }else{
                 printf("\"%s\" is received. ", buf);
                 send_base++;
             }
            rcvnum = buf[4];
            }
        }


        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }

        if (retval == 0) {
           printf("server: This host left this chatting room\n");
           break;
        }

    }

    pthread_exit(NULL);
}

void *send_packets(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in serveraddr = socket->serveraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    char rcvnum = 0;
    int retval = 0;
    int error = 1;
    while(1){

        pthread_t send_t = pthread_self();
        while(nextseqnum < send_base + WINDOWSIZE){

            sprintf(socket->buf, "%s", packets[nextseqnum]);
            buf[8] = '\0';

            if(pthread_create(&send_t, NULL, send_packet, socket)!=0){
                fprintf(stderr, "thread create error\n");
                exit(1);
            }


            if(nextseqnum<send_base + WINDOWSIZE)
                nextseqnum++;

            printf("\"%s\" is transmitted. \n", socket->buf);
            sleep(1);
        }

        if(timeout[send_base] ==5){
            sleep(1);
            printf("\"%s\" is timeout. \n", packets[send_base]);
            nextseqnum = send_base;
        }

    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int retval;
    pthread_t send_t, recv_t;

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));


    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    // 소켓 생성
    _socket->sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (_socket->sock == INVALID_SOCKET) err_quit("socket()");

    memset(&_socket->serveraddr, 0, sizeof(_socket->serveraddr));
    _socket->serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &_socket->serveraddr.sin_addr);
    _socket->serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(_socket->sock, (struct sockaddr *)&_socket->serveraddr, sizeof(_socket->serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    if(pthread_create(&send_t, NULL, send_packets, _socket)!=0){
       fprintf(stderr, "thread create error\n");
        exit(1);
    }

    if(pthread_create(&recv_t, NULL, receive, _socket)!=0){
        fprintf(stderr, "thread create error\n");
        exit(1);
    }
    pthread_join(send_t, NULL);
    pthread_join(recv_t, NULL);

    // 소켓 닫기
    close(_socket->sock);
    return 0;
}
                                                       