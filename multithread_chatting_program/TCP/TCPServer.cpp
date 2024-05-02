#include "../Common.h"
#define SERVERPORT 9000
#define BUFSIZE    512

typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in clientaddr;
} socket_sockaddrin;

void *receive(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;


    SOCKET sock = socket->sock;
    struct sockaddr_in clientaddr = socket->clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    while(1){

        int retval;
        addrlen = sizeof(clientaddr);
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }

        buf[retval] = '\0';

        if (retval == 0) { // 클라이언트가 연결을 종료한 경우
           printf("client disconnected\n");
           break; // 클라이언트 연결이 끊겼으므로 루프 종료
        }
        printf("client : %s\n", buf);
    }

    pthread_exit(NULL);
}

void *send_message(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in clientaddr = socket->clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    while(1){


        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;
        int retval;



        // '\n' 문자 제거
        int len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // 데이터 보내기
        retval = send(sock, buf, len, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("server : %s\n", buf);

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

    // accept()
    while(1){

        addrlen = sizeof(_socket->clientaddr);
        _socket->sock = accept(listen_sock, (struct sockaddr *)&_socket->clientaddr, &addrlen);
        if (_socket->sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
    }

    // 접속한 클라이언트 정보 출력



    // 데이터 통신에 사용할 변수


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
    printf("main is running");
    }
    // 소켓 닫기
    close(_socket->sock);
    return 0;
}