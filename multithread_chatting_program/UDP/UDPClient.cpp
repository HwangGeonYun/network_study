#include "../Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512


typedef struct socket_sockaddrin{
    SOCKET sock;
    sockaddr_in peeraddr;
} socket_sockaddrin;

    struct sockaddr_in peeraddr ;

void *receive(void* arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    socklen_t addrlen;
    char buf[BUFSIZE+1];

    int retval;
    while(1){
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0,
            (struct sockaddr *)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            break;
        }

        // 송신자의 주소 체크
        if (memcmp(&peeraddr, &peeraddr, sizeof(peeraddr))) {
            printf("[오류] 잘못된 데이터입니다!\n");
            break;
        }

        // 받은 데이터 출력
        buf[retval] = '\0';
        //printf("client : ", buf);;
        printf("server : %s\n", buf);
    }
    pthread_exit(NULL);
}

void *send_message(void *arg){
    socket_sockaddrin* socket = (socket_sockaddrin*)arg;

    SOCKET sock = socket->sock;
    socklen_t addrlen;
    char buf[BUFSIZE+1];

    int retval;
    int len;
    while(1){
        if ( fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;
        // '\n' 문자 제거
        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;


        // 데이터 보내기
        retval = sendto(sock, buf, (int)strlen(buf), 0,
            (struct sockaddr *)&peeraddr, sizeof(peeraddr));
        if (retval == SOCKET_ERROR) {
            err_display("sendto()");
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

    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    socket_sockaddrin* _socket = (socket_sockaddrin*)malloc(sizeof(socket_sockaddrin));

    // 소켓 생성
    _socket->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket->sock == INVALID_SOCKET) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // 데이터 통신에 사용할 변수
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    int len;

    peeraddr = serveraddr;
    // 서버와 데이터 통신
    while (1) {
        // 받은 데이터 출력
        buf[retval] = '\0';

        if(pthread_create(&send_t, NULL, send_message, _socket)!=0){
           fprintf(stderr, "thread create error\n");
            exit(1);
        }

        if(pthread_create(&recv_t, NULL, receive, _socket)!= 0){
            fprintf(stderr, "thread create error\n");
            exit(1);
        }
        pthread_join(send_t, NULL);
        pthread_join(recv_t, NULL);
    }
    // 소켓 닫기
    close(_socket->sock);
    return 0;