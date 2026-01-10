#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <vector>
const int serverPort=8888;
const char* serverIp="127.0.0.1";
const int buffSize=8192;
int main(){
    int socket_fd=socket(AF_INET,SOCK_STREAM,0);
    if(socket_fd==-1){
        std::cerr<<"create socket failed"<<strerror(errno)<<std::endl;
        return 1;
    }
    sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(serverPort);
    if (inet_pton(AF_INET, serverIp, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        close(socket_fd); 
        return 1;
    }
     std::vector <char>sendBuff(buffSize);
std::vector <char>recvBuff(buffSize);
     while(true){
        std::cout<<"please input"<<std::endl;
        std::cin.getline(sendBuff.data(),8192);
        if(strcmp(sendBuff.data(),"exit"));
        send(socket_fd,sendBuff.data(),buffSize,0);  
        memset(recvBuff.data(),buffSize,sizeof(recvBuff));
        ssize_t recv_len=recv(socket_fd,recvBuff.data(),buffSize,0);
        if(recv_len>0){
            std::cout<<"receive messige :"<<recvBuff.data()<<std::endl;
        }else if (recv_len == 0) {
            std::cout << "Server closed connection." << std::endl;
            break;
        } else {
            perror("recv failed");
            break;
        }
    }
    close(socket_fd);

}