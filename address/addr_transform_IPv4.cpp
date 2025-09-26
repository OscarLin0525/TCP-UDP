#include <stdio.h>
#include <arpa/inet.h>

int main(){
    const char *addr1 = "8.8.8.8";
    const char *addr2 = "10.17.74.66";

    /*
    Internet address
        typedef uint32_t in_addr_t; //uint32 == 32-bit unsigned integer
        struct in_addr
        {
            in_addr_t s_addr;
        }; 
    */
    in_addr_t naddr2 = inet_addr(addr2);
    in_addr_t naddr1 = inet_addr(addr1);
    // 8.8.8.8
    // bin: 00001000 00001000 00001000 00001000
    // dec: 134744072


    if(naddr1 == INADDR_NONE){
        printf("wrong address1\n");
    }
    printf("address %s, ip address = %u\n", addr1, naddr1);
    
    if(naddr2 == INADDR_NONE){
        printf("wrong address2\n");
    }
    printf("address %s, ip address = %u\n", addr2, naddr2);
    return 0;
}