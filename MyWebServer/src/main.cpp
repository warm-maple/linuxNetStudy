#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include <functional>
#include <iostream>
void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf)
{
    std::cout << "onMessage 被触发..." << std::endl;

    
    while (true)
    {
        // input_buff 变成了 buf->
        std::string msg = buf->getMes(); 
        
        size_t pos = msg.find('\n');
        
        if (pos != std::string::npos)
        {
            size_t len = pos + 1; 
            std::string one_msg = msg.substr(0, len);
            
            std::cout << "处理完整包: " << one_msg;
            
            
            conn->send(one_msg); 
            
            buf->retrecv(len); 
        }
        else
        {
            break;
        }
    }
}
int main()
{
    
    EventLoop loop;
    InetAddress addr(8888);
    TcpServer server(loop, addr);
    server.setMessageCallback(onMessage);

    loop.loop();
    return 0;
}