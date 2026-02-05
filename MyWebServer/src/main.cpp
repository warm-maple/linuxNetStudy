#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include <functional>
#include <iostream>
#include "HttpContext.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf) {
   HttpContext &context = conn->getContext();
    if (!context.parseRequest(buf)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    if (context.gotAll()) {
       const HttpRequest& req = context.request();
        std::cout << "收到请求: " << req.methodString() << " " << req.path() << std::endl;
        std::string connection = req.getHeader("Connection");
        bool close = (connection == "close") || 
                     (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
        HttpResponse response(close);
        if (req.path() == "/") {
            response.setStatusCode(HttpResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType("text/html");
            response.setBody("<html><body><h1>Hello, High Performance Server!</h1></body></html>");
        } 
        else if (req.path() == "/hello") {
            response.setStatusCode(HttpResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType("text/plain");
            response.setBody("Hello World!");
        }
        else {
            response.setStatusCode(HttpResponse::k404NotFound);
            response.setStatusMessage("Not Found");
            response.setBody("404 Sorry, not found.");
        }
        Buffer output;
        response.appendToBuffer(&output);
        conn->send(output.getMes()); 
        if (response.closeConnection()) {
          conn->shutdown(); 
        }
        context.reset();
    }
}
int main()
{
    
    EventLoop loop;
    InetAddress addr(8888);
    TcpServer server(&loop, addr);
    server.setThreadNum(4);  
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
    return 0;
}