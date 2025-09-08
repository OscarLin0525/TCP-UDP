#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define serverPort 48763

char *convert(char *src){
    char *iter = src;
    char *result = new char[strlen(src) + 1];
    char *it = result;
    if(iter == NULL){
        return iter;
    }
    while(*iter){
        // a = 97(10) = 0x01100001
        // A = 65(10) = 0x01000001 = a + 32 [and(&) ~0x00100000]
        *it++ = *iter++ & ~0x20; // 0x00000020(16) == 32(10) transform to uppercase
    }
    *it = '\0';
    return result;
}

int main(){
    char buf[1024] = {0};
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0){
        printf("Fail to create a socket.");
    }

    // Correct, universally compatible way for the client
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); // Good practice to zero out the struct first

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Assign members one by one

    
    // bind the socket to assign port 
    if (bind(socket_fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    printf("Server ready!\n");

    struct  sockaddr_in clientAddr;
    u_int32_t len = sizeof(clientAddr);
    while(1){
        if(recvfrom(socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &len) < 0){
            break;
        }

        if(strcmp(buf, "exit") == 0){
            printf("Exit command received. Shutting down server.\n");
            break;

        }

        char *conv = convert(buf);
        // transform bin to human readable ip address(ex: 192.168.0.3)
        printf("get message from [%s:%d]: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        sendto(socket_fd, conv, strlen(conv), 0, (struct sockaddr *)&clientAddr, len);
        memset(buf, 0, sizeof(buf));
        free(conv);
    }

    if(socket_fd < 0){
        perror("close socket failed!");
    }
    return 0;
}
