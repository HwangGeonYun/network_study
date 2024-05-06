#include "../Common.h"
#define SERVERPORT 9000
#define BUFSIZE    512

const char* ACKS[6] = {"ACK 0", "ACK 1", "ACK 2", "ACK 3", "ACK 4", "ACK 5"};



typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in clientaddr;
    char buf[BUFSIZE + 1];
} socket_sockaddrin;

int rcv_base = 0;
int missed_max = -1;
pthread_mutex_t mutex;


void *rcv_packet_send_ACK(void *arg){

    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in clientaddr = socket->clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    int _rcv_base = 0;

    int retval;

    int received_num = socket->buf[7] - '0';
    printf("\"%s\" is received", socket->buf);



    if(rcv_base < received_num){
        printf(" and dropped. \"%s\" is retransmitted.\n", ACKS[rcv_base-1]);
        missed_max = received_num;
    }
    else{
        if(received_num<=missed_max)
           printf(" and delivered");
        printf(". \"%s\" is transmitted. \n", ACKS[rcv_base]);
           rcv_base++;
    }

    _rcv_base = rcv_base;

    sleep(4);       


    pthread_mutex_lock(&mutex);
    sprintf(buf, "%s", ACKS[_rcv_base-1]);
    send(sock, buf, BUFSIZE, 0);
    pthread_mutex_unlock(&mutex);


    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int retval;

    pthread_t send_t, recv_t = pthread_self();

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));

    SOCKET listen_sock;

    struct sockaddr_in clientaddr = _socket->clientaddr;
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

    // accept()
    while(1){

        addrlen = sizeof(clientaddr);
        _socket->sock = accept(listen_sock, (struct sockaddr *)&_socket->clientaddr, &addrlen);
        if (_socket->sock == INVALID_SOCKET) {
            err_display("accept()");
            break;    
    }

    // 접속한 클라이언트 정보 출력



    // 데이터 통신에 사용할 변수

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
        if(pthread_create(&recv_t, NULL, rcv_packet_send_ACK, _socket)!=0){
            fprintf(stderr, "thread create error\n");
            exit(1);
        }

    }




    }
    // 소켓 닫기
    close(_socket->sock);
    return 0;
}
                      
                        