#include "../Common.h"
#define SERVERPORT 9000
#define BUFSIZE    512

const char* ACKS[6] = {"ACK 0", "ACK 1", "ACK 2", "ACK 3", "ACK 4", "ACK 5"};
char* buffered_packet[6];

typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in clientaddr;
    char buf[BUFSIZE + 1];
} socket_sockaddrin;

int rcv_base = 0;
int buffered_max = -1;
pthread_mutex_t mutex;


void *_receive(void *arg){

    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in clientaddr = socket->clientaddr;
    socklen_t addrlen;
    char rcvbuf[BUFSIZE + 1];
    char sendbuf[BUFSIZE + 1];
    int _rcv_base = 0;

    int retval;

    int received_num = socket->buf[7] - '0';

    sprintf(rcvbuf, "%s", socket->buf);
    printf("\"%s\" is received", rcvbuf);

    if(rcv_base < received_num){
        printf(" and buffered");
        buffered_max = received_num;
        buffered_packet[received_num] = rcvbuf;
        //printf(" %d %s",received_num, buffered_packet[received_num]);
        printf(". \"%s\" is retransmitted.\n", ACKS[received_num]);
    }
    else{
        if(rcv_base < buffered_max){
            buffered_packet[rcv_base] = rcvbuf;
            printf(". ");

            //printf("%s ", buffered_packet[4]);
            for(int i=rcv_base;i<=buffered_max;i++){
                if(i != buffered_max)
                    printf("%s, ",buffered_packet[i]);
                else
                    printf("and %s",buffered_packet[i]);
            }
            printf(" are delivered");
        }
        rcv_base++;
        printf(". \"%s\" is transmitted.\n", ACKS[received_num]);
    }


            /*for(int i=rcv_base;i<=buffered_max;i++){
                printf("%s ", buffered_packet[i]);
            }*/
    _rcv_base = rcv_base;

    sleep(4);


    pthread_mutex_lock(&mutex);
    sprintf(sendbuf, "%s", ACKS[received_num]);
    send(sock, sendbuf, BUFSIZE, 0);
    //sprintf(buf, "hello");
    pthread_mutex_unlock(&mutex);


    pthread_exit(NULL);
}

void *receive(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;


    SOCKET sock = socket->sock;
    struct sockaddr_in clientaddr = socket->clientaddr;
    socklen_t addrlen;
    int missed_max = 0;
    int first = 1;
    while(1){
        int retval;
        retval = recv(sock, socket->buf, BUFSIZE, 0);
        if (retval == 0) { // 클라이언트가 연결을 종료한 경우
           printf("client disconnected\n");
           break; // 클라이언트 연결이 끊겼으므로 루프 종료
        }

        //socket->;

        pthread_t recv_t = pthread_self();
        if(pthread_create(&recv_t, NULL, _receive, socket)!=0){
            fprintf(stderr, "thread create error\n");
            exit(1);
        }

    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int retval;

    pthread_t send_t, recv_t = pthread_self();

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));

    SOCKET listen_sock;

    // 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
       struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    socklen_t addrlen;

    while(1){

        addrlen = sizeof(_socket->clientaddr);
        _socket->sock = accept(listen_sock, (struct sockaddr *)&_socket->clientaddr, &addrlen);
        if (_socket->sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        SOCKET sock = _socket->sock;

        while(1){
            int retval;
            retval = recv(sock, _socket->buf, BUFSIZE, 0);
            if (retval == 0) { // 클라이언트가 연결을 종료한 경우
               printf("client disconnected\n");
               break; // 클라이언트 연결이 끊겼으므로 루프 종료
            }

            //socket->;

            pthread_t recv_t = pthread_self();
            if(pthread_create(&recv_t, NULL, _receive, _socket)!=0){
                fprintf(stderr, "thread create error\n");
                exit(1);
            }

        }
                      
    }

    // 소켓 닫기
    close(_socket->sock);
    return 0;
}