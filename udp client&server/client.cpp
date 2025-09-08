#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define serverIP "10.17.74.66"
#define serverPort 48763

char buf[1024] = {0};
char recvbuf[1024] = {0};

int main(){
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0){
        printf("Create socket fail!\n");
        return -1;
    }

    // Correct, universally compatible way for the client
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); // Good practice to zero out the struct first

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); // Assign members one by one

    
    uint32_t len = sizeof(serverAddr);

    while(1){
        printf("Please input your message: ");
        scanf("%s", buf);

        sendto(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

        if(strcmp(buf, "exit") == 0)
            break;

        memset(buf, 0, sizeof(buf));

        if(recvfrom(socket_fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&serverAddr, &len) < 0){
            printf("recvfrom data from %s:%d, failed!\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            break;
        }

        printf("get receive message from [%s:%d]: %s\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    if(close(socket_fd) < 0){
        perror("close socket failed!");
        return -1;
    }

    return 0;
}
