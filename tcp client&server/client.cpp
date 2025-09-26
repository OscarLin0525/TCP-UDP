#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define serverPort 45678
#define serverIP "127.0.0.1"
#define BUF_SIZE 1024

int main(){
    char buf[BUF_SIZE] = {0};
    char recvbuf[BUF_SIZE] = {0};

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("socket() error");
        return -1;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET,
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    int len = sizeof(serverAddr);

    if(connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1){
        perror("connect() error");
        close(socket_fd);
        exit(0);
    }

    printf("connect to server %s:%d successfully!\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while (1)
    {
        printf("Please input message: ");
        scanf("%s", buf);

        if(send(socket_fd, buf, sizeof(buf), 0) < 0){
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            break;
        }

        if(strcmp(buf, "exit") == 0){
            break;
        }

        memset(buf, 0, sizeof(buf));

        if(recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0){
            printf("recv data from %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            memset(recvbuf, 0, sizeof(recvbuf));
            break;
        }

        printf("get receive message from [%s:%d]: %s\n", 
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    if(close(socket_fd) < 0){
        perror("close socket failed!");
    }
    return 0;
}