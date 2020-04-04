#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_map>

const uint16_t PORT = 3425;
const size_t BUFF_SIZE = 1024;
const int CONNECTIONS_NUM = 10;
using set_iter = std::set<int>::iterator;

int main(int argc, char** argv) {
    int listener;
    struct sockaddr_in addr;
    char buf[BUFF_SIZE];
    
    // создаём ip-socket
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("socket");
        exit(1);
    }
    // делаем сокет неблокирующим
    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = htons(PORT); // порт
    addr.sin_addr.s_addr = INADDR_ANY; // принимаем любое сообщение

    //  связываем сокет с адресом и номером порта 
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }
    // выражаем готовность принимать входящие соединения и
    // задаём размер очереди
    listen(listener, CONNECTIONS_NUM);
    
    std::set<int> clients;
    clients.clear();
    std::unordered_map<int, sockaddr_in> addresses;
    addresses.clear();

    std::ofstream out("statistics.txt", std::ios::app);
    
    std::cout << "Server started at: " << inet_ntoa(addr.sin_addr) 
        << ':' << PORT << std::endl;

    while(true) {
        // Заполняем множество сокетов
        fd_set readset;
        FD_ZERO(&readset); // очищаем множество
        FD_SET(listener, &readset); // добавляем дескриптор сокета в множество

        for(set_iter it = clients.begin(); it != clients.end(); it++)
            FD_SET(*it, &readset); // добавляем дескриптор клиента в множество

        // Задаём таймаут
        timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        // Ждём события в одном из сокетов
        int mx = std::max(listener, *max_element(clients.begin(), clients.end()));
        // в readset записываются дескрипторы сокетов из которых нам требуется читать данные
        // mx+1 максимальное значение дескриптора + 1
        if(select(mx+1, &readset, NULL, NULL, &timeout) <= 0) {
            perror("select");
            exit(3);
        }
        
        // Определяем тип события и выполняем соответствующие действия
        // Проверяем содержится ли дескриптор listener в множестве readset
        if(FD_ISSET(listener, &readset)) {
            // Поступил новый запрос на соединение, используем accept
            sockaddr_in cs_addr;
            socklen_t cs_addrsize = sizeof (cs_addr);
  
            int client_socket= accept(listener, (struct sockaddr *) &cs_addr,  &cs_addrsize);
            if(client_socket < 0) {
                perror("accept");
                exit(3);
            }
            // делаем неблокирующим
            fcntl(client_socket, F_SETFL, O_NONBLOCK);

            clients.insert(client_socket);
            addresses[client_socket] = cs_addr;
        }

        for(set_iter it = clients.begin(); it != clients.end(); it++) {
            if(FD_ISSET(*it, &readset)) {
                // чистим буфер
                memset(buf, 0, sizeof(char)*BUFF_SIZE);
                // Поступили данные от клиента, читаем их
                ssize_t bytes_read = recv(*it, buf, BUFF_SIZE, 0);

                if(bytes_read <= 0) {
                    // Соединение разорвано, удаляем сокет из множества
                    close(*it); // закрываем сокет
                    clients.erase(*it);
                    continue;
                }

                sockaddr_in addr = addresses[*it];

                std::cout << inet_ntoa(addr.sin_addr) << ':' << addr.sin_port 
                    << ' ' << buf << std::endl;

                if (out.is_open()) {
                    out << "RECIEVE " << inet_ntoa(addr.sin_addr) << ':' << addr.sin_port 
                    << ' ' << buf << std::endl;
                }
                if (out.is_open()) {
                    out << "SEND TO " << inet_ntoa(addr.sin_addr) << ':' << addr.sin_port 
                    << ' ' << buf << std::endl;
                }
                // Отправляем данные обратно клиенту
                send(*it, buf, bytes_read, 0);
            }
        }
    }
    
    out.close();
    return EXIT_SUCCESS;
}