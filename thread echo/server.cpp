#include <iostream>
#include <cstring>      // memset, strerror
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_ntop
#include <unistd.h>     // close, read, write
#include <thread>
#include <mutex>
const int PORT = 8888;
const int buffSize = 8192;
std::mutex print_mut;

void handle(int client_fd, sockaddr_in client_addr)
{
    char client_ip[16];
    memset(client_ip, 0, sizeof(client_ip));
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, 16);
    std::cout << "Client connected: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
    char buff[buffSize];
    while (true)
    {
        memset(buff, 0, sizeof(buff));
        ssize_t msg = recv(client_fd, buff, sizeof(buff) - 1, 0);
        if (msg > 0)
        {
            {
                std::lock_guard<std::mutex> lock(print_mut);
                std::cout << "[" << client_ip << "]: " << buff << std::endl;
            }
            send(client_fd, buff, msg, 0);
        }
        else if (msg == 0)
        {
            {
                std::lock_guard<std::mutex> lock(print_mut);
                std::cout << "Client disconnected." << std::endl;
            }
            break;
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock(print_mut);
                std::cerr << "Recv error: " << strerror(errno) << std::endl;
            }
            break;
        }
    }
    close(client_fd);
    std::cout << "Waiting for next connection..." << std::endl;
}
int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        std::cerr << "create socket failed" << strerror(errno) << std::endl;
        return 1;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "bind failed" << strerror(errno) << std::endl;
        return 1;
    }
    if (listen(listen_fd, 128) == -1)
    {
        std::cerr << "listen failed" << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "----------server is listening ------------------------" << std::endl;
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t clientlen = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &clientlen);
        if (client_fd == -1)
        {
            std::cerr << "accept failed" << strerror(errno) << std::endl;
            continue;
        }
        std::thread p(handle, client_fd, client_addr);
        p.detach();
    }
}