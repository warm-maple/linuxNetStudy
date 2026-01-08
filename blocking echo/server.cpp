#include <iostream>
#include <cstring>      // memset, strerror
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_ntop
#include <unistd.h>     // close, read, write
const int PORT = 8888;
const int buffSize = 1024;
int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        std::cerr << "Create socket failed" << strerror(errno) << std::endl;
        return 1;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cout << "bind failed" << strerror(errno) << std::endl;
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, 128) == -1)
    {
        std::cerr << "listen failed" << strerror(errno) << std::endl;
        close(listen_fd);
        return 1;
    }
    std::cout << "Server is listening on port" << PORT << std::endl;
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1)
        {
            std::cerr << "accept failed" << strerror(errno) << std::endl;
            continue;
        }
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        char buff[buffSize];
        while (true)
        {
            memset(&buff, 0, sizeof(buff));
            ssize_t msg = recv(client_fd, buff, sizeof(buff) - 1, 0);
            if (msg > 0)
            {
                std::cout << "Received: " << buff << std::endl;
                send(client_fd, buff, msg, 0);
            }
            else if (msg == 0)
            {
                std::cout << "Client disconnected." << std::endl;
                break;
            }
            else
            {
                std::cerr << "Recv error: " << strerror(errno) << std::endl;
                break;
            }
        }
        close(client_fd);
        std::cout << "Waiting for next connection..." << std::endl;
    }
    close(listen_fd);
    return 0;
}