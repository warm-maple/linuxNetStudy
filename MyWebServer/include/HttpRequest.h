#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <iostream>

class HttpRequest {
public:
    enum Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
    enum Version { kUnknown, kHttp10, kHttp11 };

    HttpRequest() : method_(kInvalid), version_(kUnknown) {}

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    bool setMethod(const char* start, const char* end) {
        std::string m(start, end);
        if (m == "GET") method_ = kGet;
        else if (m == "POST") method_ = kPost;
        else if (m == "HEAD") method_ = kHead;
        else method_ = kInvalid;
        return method_ != kInvalid;
    }
    Method getMethod() const { return method_; }

    void setPath(const char* start, const char* end) { path_.assign(start, end); }
    const std::string& path() const { return path_; }

    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) { ++colon; } // 去掉冒号后的空格
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1])) { value.pop_back(); } // 去掉末尾空格
        headers_[field] = value;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        auto it = headers_.find(field);
        if (it != headers_.end()) { result = it->second; }
        return result;
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::map<std::string, std::string> headers_;
};

#endif