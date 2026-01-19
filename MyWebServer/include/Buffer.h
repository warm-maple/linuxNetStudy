// Buffer.h - declaration only
#pragma once

#include <vector>
#include <string>
#include <sys/types.h>

class Buffer {
public:
    Buffer();
    ~Buffer() = default;

    size_t size() const;
    void buffclear();

    // 返回消息的拷贝（用于调试/打印）
    std::string getMes() const;

    // 从 socket 读取数据，返回读取到的字节数，0 表示对端关闭，-1 表示错误
    ssize_t read(int client_fd);

    // 零拷贝访问（返回当前未读数据的指针和长度）
    const char* data() const;
    size_t getBuffRead();
    size_t getReadLen();
    void write(const char *buf, ssize_t len);
    void retrecv(size_t len);
private:
    std::vector<char> buff;
    size_t first;
    size_t last;
};