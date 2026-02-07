#include "HttpContext.h"
#include <algorithm> 
#include <cstring>   


bool HttpContext::parseRequest(Buffer* buf) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (buf->readableBytes() == 0) {
            break;
        }
        if (state_ == kExpectRequestLine) {
            const char* crlf = std::search(buf->peek(), buf->peek() + buf->readableBytes(), "\r\n", "\r\n" + 2);
            
            if (crlf != buf->peek() + buf->readableBytes()) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    buf->retrieveUntil(crlf + 2); 
                    state_ = kExpectHeaders;      
                } else {
                    hasMore = false; 
                }
            } else {
                hasMore = false;
            }
        }
        
        else if (state_ == kExpectHeaders) {
            const char* crlf = std::search(buf->peek(), buf->peek() + buf->readableBytes(), "\r\n", "\r\n" + 2);
            
            if (crlf != buf->peek() + buf->readableBytes()) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2); 
            } else {
                hasMore = false; 
            }
        }
        else if (state_ == kExpectBody) {
            
            hasMore = false;
        }
    }
    return ok;
}

// 解析 "GET /index.html HTTP/1.1"
bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    
   
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        
        space = std::find(start, end, ' ');
        if (space != end) {
            request_.setPath(start, space);
            start = space + 1;
            
            std::string version(start, end);
            if (version == "HTTP/1.1") {
                request_.setVersion(HttpRequest::kHttp11);
                succeed = true;
            } else if (version == "HTTP/1.0") {
                request_.setVersion(HttpRequest::kHttp10);
                succeed = true;
            }
        }
    }
    return succeed;
}