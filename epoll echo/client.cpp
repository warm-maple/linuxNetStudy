#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
const int serverPort = 8888;
const char *serverIp = "127.0.0.1";
const int buffSize = 8192;
int main()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        std::cerr << "create socket failed" << strerror(errno) << std::endl;
        return 1;
    }
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIp, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        return 1;
    }
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection Failed");
        close(socket_fd);
        return 1;
    }
    std::string send_buff;
    std::vector<char> recvBuff(8192);
    while (true)
    {
        std::cout << "please input" << std::endl;
        std::getline(std::cin, send_buff);
        if (send_buff == "exit")break;
        send(socket_fd, send_buff.data(), send_buff.length(), 0);
        recvBuff.clear();
        char buf[1024];
        ssize_t read_len ;
        int retry_count = 0;
const int MAX_RETRIES = 100;
while (retry_count < MAX_RETRIES) {
    memset(buf, 0, 1024);
    read_len = recv(socket_fd, buf, 1023, MSG_DONTWAIT);
    if (read_len > 0) {
        recvBuff.insert(recvBuff.end(), buf, buf + read_len);
        retry_count = 0; 
    } else if (read_len == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (!recvBuff.empty()) {
                
                usleep(1000);
                retry_count++;
                continue;
            } else {
                
                usleep(1000);
                retry_count++;
                continue;
            }
        } else {
            perror("recv");
            close(socket_fd);
            return 1;
        }
    } else { 
        std::cout << "Server closed connection" << std::endl;
        close(socket_fd);
        return 0;
    }
}
if (recvBuff.empty()) {
    std::cout << "No response from server (timeout)" << std::endl;
}
        if (!recvBuff.empty())
        {   std::string recv(recvBuff.begin(),recvBuff.end());
            std::cout << "receive messige :" << recv << std::endl;
        }
    }
    close(socket_fd);
    recvBuff.clear();
}