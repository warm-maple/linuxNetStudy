#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
int main()
{
    EventLoop loop;
    InetAddress addr(8888);
    TcpServer server(loop, addr);
    loop.loop();
    return 0;
}