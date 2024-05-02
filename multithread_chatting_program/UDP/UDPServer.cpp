#include "../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

        struct sockaddr_in clientaddr;

typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in clientaddr;
}socket_sockaddrin;

void* receive(void * arg){
        socket_sockaddrin* socket = (socket_sockaddrin*)arg;

        SOCKET sock = socket->sock;
        socklen_t addrlen;
        char buf[BUFSIZE+1];
        int retval;
       // int retval;
        while(1){

            addrlen = sizeof(clientaddr);
            retval = recvfrom(sock, buf, BUFSIZE, 0,
                (struct sockaddr *)&clientaddr, &addrlen);
           if (retval == SOCKET_ERROR) {
                err_display("recvfrom()");
                break;
            }

        // 받은 데이터 출력
            buf[retval] = '\0';
            char addr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
            printf("client : %s\n", buf);
        }


        pthread_exit(NULL);
}

void* send_message(void *arg){
        socket_sockaddrin* socket = (socket_sockaddrin*)arg;

        SOCKET sock = socket->sock;
        socklen_t addrlen;
        char buf[BUFSIZE+1];

        int retval;


        while(1){
            if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
                break;

            retval = sendto(sock, buf, (int)strlen(buf), 0,
                (struct sockaddr *)&clientaddr, sizeof(clientaddr));
            if (retval == SOCKET_ERROR) {
                err_display("sendto()");
                break;
            }

            printf("server : %s", buf);
        }
        pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int retval;

    pthread_t send_t, recv_t;

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));

    // 소켓 생성
    _socket->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket->sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(_socket->sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 데이터 통신에 사용할 변수
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트와 데이터 통신
    while (1) {

        if(pthread_create(&send_t, NULL, send_message, _socket)!=0){
           fprintf(stderr, "thread create error\n");
            exit(1);
        }

        if(pthread_create(&recv_t, NULL, receive, _socket)!=0){
            fprintf(stderr, "thread create error\n");
            exit(1);
        }


        pthread_join(send_t, NULL);
        pthread_join(recv_t, NULL);
    }

    // 소켓 닫기
    close(_socket->sock);
    return 0;
}