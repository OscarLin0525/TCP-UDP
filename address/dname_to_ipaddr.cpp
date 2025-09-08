#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

//set the target server
#define ntu "www.ntu.edu.tw"


int main(){
    struct addrinfo form
    {
        form.ai_family = AF_UNSPEC, // both IPv4 and IPv6 are acceptable
        form.ai_socktype = SOCK_STREAM, // TCP socket
        form.ai_flags = AI_PASSIVE, // server mode
    };
    struct  addrinfo *serverInfo, *iter;
    int status = 0;

    // get server information
    status = getaddrinfo(ntu, "443", &form, &serverInfo);

    if (status != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(0);
    }

    for(iter = serverInfo; iter!=NULL; iter = iter->ai_next){
        void *addr;
        uint16_t port;
        char ip_address[INET6_ADDRSTRLEN];
        switch (iter->ai_family){
            case AF_INET:{
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter->ai_addr;
                addr = &(ipv4->sin_addr);
                port = ntohs(ipv4->sin_port);
                printf("addr: %u,port: %d\n", addr, port);
                break;
            }
            case AF_INET6:{
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)iter->ai_addr;
                addr = &(ipv6->sin6_addr);
                port = ntohs(ipv6->sin6_port);
                printf("addr: %u,port: %d\n", addr, port);
                break;
            }
        }
        inet_ntop(iter->ai_family, addr, ip_address, sizeof(ip_address));
        printf("%s -> %s : %d\n", ntu, ip_address, port);
    }
    freeaddrinfo(serverInfo);
    return 0;
}