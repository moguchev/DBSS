#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

const uint16_t PORT = 3425;
const size_t BUFF_SIZE = 1024;
char buf[BUFF_SIZE];

int main(int argc, char** argv)
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = htons(PORT); // порт
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // host - 127.0.0.1
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(2);
    }

    std::string message;
    while(true) {
        std::cin >> message;
        if (message == "END") {
            break;
        }
        send(sock, message.c_str(), message.size() * sizeof(char), 0);
        recv(sock, buf, sizeof(buf), 0);
        std::cout << buf << std::endl;
        memset(buf, 0, sizeof(char)*BUFF_SIZE);
    }
   
    close(sock);

    return EXIT_SUCCESS;
}