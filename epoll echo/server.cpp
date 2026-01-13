#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h> //Epoll 头文件
#include <fcntl.h>
#include <errno.h>
const int buffSize = 8192;
const int PORT = 8888;
const int MAX_EVENTS = 1000;
void setnonlock(int client_fd)
{
    int old_op = fcntl(client_fd, F_GETFL);
    int new_op = old_op | O_NONBLOCK;
    fcntl(client_fd, F_SETFL, new_op);
}
int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        std::cout << "create socket failed" << std::endl;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "bind failed" << strerror(errno) << std::endl;
        return 1;
    }
    if (listen(listen_fd, 128) == -1)
    {
        perror("listen");
        return 1;
    }
    std::cout << "------------------------server is listening on 8888-----------------------------" << std::endl;
    int epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll_create1");
        return 1;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1)
    {
        perror("epoll_ctl");
        return 1;
    }
    std::vector<struct epoll_event> events(MAX_EVENTS);
    while (true)
    {
        int n = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);
        if (n == -1)
        {
            perror("epoll_wait");
            return 1;
        }
        for (int i = 0; i < n; i++)
        {
            int client_fd = events[i].data.fd;
            if (client_fd == listen_fd)
            {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                    perror("accpet");
                    return 1;
                }
                setnonlock(client_fd);
                char client_ip[16];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                std::cout << "New Client: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
                epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    perror("epoll_ctl");
                    return 1;
                    close(client_fd);
                }
            }
            else
            {    if (events[i].events & EPOLLIN)
                {
                    std::vector<char> buff;
                    char buf[1024];
                    ssize_t read_len;
                    while (true)
                    {
                        memset(buf, 0, sizeof(buf));
                        read_len = recv(client_fd, buf, 1023, 0);
                        if (read_len > 0)
                        {
                            buff.insert(buff.end(), buf, buf + read_len);
                        }
                        else if (read_len == -1)
                        {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                std::cout << "data finish,EAGAIN" << std::endl;
                                break;
                            }
                            else
                            {
                                perror("recv");
                                close(client_fd);
                                buff.clear();
                                break;
                            }
                        }
                        else if (read_len == 0)
                        {
                            close(client_fd);
                            buff.clear();
                            break;
                        }
                    }

                    if (!buff.empty())
                    {
                        std::cout << "receive messige from " << client_fd <<"the length of the message is "<< buff.size()<< std::endl;
                        std::cout << "Recv full msg:";
                        std::cout.write(buff.data(), buff.size());
                        std::cout << std::endl;
                        send(client_fd, buff.data(), buff.size(), 0);
                    }
                }else if (events[i].events & (EPOLLERR | EPOLLHUP))
                {
                    std::cout << "Client error/close fd: " << client_fd << std::endl;
                    close(client_fd);
                    continue;
                }                             
            }
        }
    }
    close(listen_fd);
    close(epfd);
    return 0;
}