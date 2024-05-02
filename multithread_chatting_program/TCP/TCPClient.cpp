#include "../Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in serveraddr;
} socket_sockaddrin;

void *receive(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in serveraddr = socket->serveraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    while(1){

        int retval;
        addrlen = sizeof(serveraddr);
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }


        if (retval == 0) {
           printf("server: This host left this chatting room\n");
           break; // 클라이언트 연결이 끊겼으므로 루프 종료
        }

        buf[retval] = '\0';
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &serveraddr.sin_addr, addr, sizeof(addr));


        buf[retval] = '\0';
        printf("server : %s\n", buf);
    }

    pthread_exit(NULL);
}

void *send_message(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    struct sockaddr_in serveraddr = socket->serveraddr;
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
        printf("client : %s\n", buf);

    }
    pthread_exit(NULL);
}



int main(int argc, char *argv[])
{
    int retval;
    pthread_t send_t, recv_t;

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));

    //SOCKET listen_sock;

    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    // 소켓 생성
    _socket->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket->sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    //struct sockaddr_in serveraddr;
    memset(&_socket->serveraddr, 0, sizeof(_socket->serveraddr));
    _socket->serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &_socket->serveraddr.sin_addr);
    _socket->serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(_socket->sock, (struct sockaddr *)&_socket->serveraddr, sizeof(_socket->serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE + 1];
    int len;

    // 서버와 데이터 통신
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

    // 소켓 닫기
    close(_socket->sock);
    return 0;
}