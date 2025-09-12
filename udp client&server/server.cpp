#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm> // for std::transform
#include <cctype>
#include <string>

#define SERVER_PORT 48763
#define BUF_SIZE 1024
// old version to implement the toupper() function. align to the low level concept 
// single instruction is very silghtly faster than function call
/*
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
    // function can not just free the source itself, it should be freed by the caller 
    // free(result);
    return result;
}
*/
// modern version to implement the toupper() function
std::string convert(const std::string& input_str){
    std::string result = input_str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return result;
}

int main(){
    char buf[BUF_SIZE] = {0};
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0){
        printf("Fail to create a socket.");
    }

    // Correct, universally compatible way for the client
    struct sockaddr_in serverAddr = {};
    // memset(&serverAddr, 0, sizeof(serverAddr)); // Good practice to zero out the struct first

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Assign members one by one

    
    // bind the socket to assign port 
    if (bind(socket_fd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    printf("Server ready!\n");

    struct sockaddr_in clientAddr;
    u_int32_t len = sizeof(clientAddr);
    while(1){
        // recvfrom() is a blocking function
        int bytes_received = recvfrom(socket_fd, buf, sizeof(buf)-1, 0, (struct sockaddr *)&clientAddr, &len);
        if(bytes_received < 0){
            perror("Receive data failed!");
            break;
        }

        std::string message(buf, bytes_received);
        if(message == "exit"){
            printf("Exit command received. Shutting down server.\n");
            break;

        }

        std::string conv = convert(message);
        // transform bin to human readable ip address(ex: 192.168.0.3)
        char client_ip[INET_ADDRSTRLEN] = {0};
        printf("get message from [%s:%d]: ", inet_ntop(AF_INET, &clientAddr.sin_addr, client_ip, INET_ADDRSTRLEN), ntohs(clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv.c_str());

        sendto(socket_fd, conv.c_str(), conv.size(), 0, (struct sockaddr *)&clientAddr, len);
        memset(buf, 0, sizeof(buf));
        // do not manually free the allocated memory 
        // free(conv);
    }
    close(socket_fd);
    return 0;
}
