#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h> //Epoll 头文件
#include <fcntl.h>
#include <errno.h>
const int buffSize=8192;
const int PORT=8888;
const int MAX_EVENTS = 1000;
void setnonlock(int client_fd){
    int old_op=fcntl(client_fd,F_GETFL);
    int new_op=old_op|O_NONBLOCK;
    fcntl(client_fd,F_SETFL,new_op);
}
int main(){
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1){std::cout<<"create socket failed"<<std::endl;}
  int opt=1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(server_fd,(sockaddr*)&server_addr,sizeof(server_addr))==-1){
        std::cerr<<"bind failed"<<strerror(errno)<<std::endl;
    return 1;}
    if(listen(server_fd,128)==-1){perror("listen");return 1;}
    std::cout<<"------------------------server is listening on 8888-----------------------------"<<std::endl;
    

}