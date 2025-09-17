#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <algorithm>

#define serverPort 45678
#define BACKLOG 5
#define BUF_SIZE 1024

pthread_t clients[BACKLOG];
pthread_t tid;


std::string convert(const std::string& input_str){
    std::string result = input_str;
    std::transform(result.begin(), result.end(), result.begin(),[](unsigned char c) { return std::toupper(c); });
    return result;
}


struct client_info{
    int sockfd;
    struct sockaddr_in clientAddr;
};

void* clientSocket(void *param){
    char buf[BUF_SIZE];
    struct client_info * info = (client_info*)param;
    
    while(recv(info->sockfd, buf, sizeof(buf), 0)){
        if(strcmp(buf, "exit") == 0){
            memset(buf, 0, sizeof(buf));
            break;
        }

        char *conv = (char*)malloc(sizeof(buf));
        memcpy(conv, convert(buf).c_str(), sizeof(buf));

         printf("get message from [%s:%d]: ",
                inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        if(send(info->sockfd, conv, sizeof(conv), 0) < 0){
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            free(conv);
            break;
        }

        memset(buf, 0, sizeof(buf));
        free(conv);
    }

    if(close(info->sockfd) < 0){
        perror("close socket fail");
    }
    else{
        printf("Socket closed from %s:%d success!\n", 
            inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
    }
}

int main(){
    char buf[BUF_SIZE];
    int client_index = 0;
    
}