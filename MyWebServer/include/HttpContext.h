#ifndef HTTP_CONTEXT_H
#define HTTP_CONTEXT_H

#include "HttpRequest.h"
#include "Buffer.h" 

class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine, 
        kExpectHeaders,     
        kExpectBody,       
        kGotAll,           
    };

    HttpContext() : state_(kExpectRequestLine) {}
    bool parseRequest(Buffer* buf);
    const HttpRequest& request() const { return request_; }
    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_ = dummy;
    }
    bool gotAll() const { return state_ == kGotAll; }

private:
    
    bool processRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

#endif