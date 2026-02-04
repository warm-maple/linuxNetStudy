
#include "../include/HttpResponse.h"
#include <cstdio>
#include "Buffer.h"
#include <string.h>
void HttpResponse::appendToBuffer(Buffer* outputBuffer) const {
    char buf[32];
    
    
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    outputBuffer->write(buf, static_cast<ssize_t>(strlen(buf)));
    outputBuffer->write(statusMessage_.c_str(), static_cast<ssize_t>(statusMessage_.size()));
    outputBuffer->write("\r\n", 2);

    // 始终添加 Content-Length 以便客户端正确解析响应
    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
    outputBuffer->write(buf, static_cast<ssize_t>(strlen(buf)));

    if (closeConnection_) {
        outputBuffer->write("Connection: close\r\n", 19);
    } else {
        outputBuffer->write("Connection: Keep-Alive\r\n", 24);
    }

   
    for (const auto& header : headers_) {
        outputBuffer->write(header.first.c_str(), static_cast<ssize_t>(header.first.size()));
        outputBuffer->write(": ", 2);
        outputBuffer->write(header.second.c_str(), static_cast<ssize_t>(header.second.size()));
        outputBuffer->write("\r\n", 2);
    }

   
    outputBuffer->write("\r\n", 2);

    
    outputBuffer->write(body_.c_str(), static_cast<ssize_t>(body_.size()));
}