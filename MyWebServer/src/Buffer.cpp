// Buffer.cpp - implementations
#include "../include/Buffer.h"
#include <cstring>
#include <sys/socket.h>
#include <errno.h>

Buffer::Buffer() : first(0), last(0) { buff.reserve(8192); }

size_t Buffer::size() const { return buff.size(); }

void Buffer::buffclear() { buff.clear(); first = last = 0; }

std::string Buffer::getMes() const { return (last > first) ? std::string(buff.begin() + first, buff.begin() + last) : std::string(); }

size_t Buffer::getReadLen() { return (last >= first) ? (last - first) : 0; }

void Buffer::write(const char *buf, ssize_t len) {
    if (len <= 0) return;
    // Append at the end
    buff.insert(buff.end(), buf, buf + len);
    last = buff.size();
}

ssize_t Buffer::read(int client_fd)
{
    ssize_t read_len;
    while (true)
    {
        char temp_buf[1024];
        memset(temp_buf, 0, sizeof(temp_buf));
        read_len = recv(client_fd, temp_buf, 1023, 0);
        if (read_len > 0)
        {
            write(temp_buf, read_len);
        }
        else if (read_len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return -1;
        }
        else if (read_len == 0)
        {
            return 0;
        }
    }
    return static_cast<ssize_t>(getReadLen());
}

size_t Buffer::getBuffRead() { return first; }

void Buffer::retrecv(size_t len){
    if (len == 0) return;
    first += len;
    if (first >= last) {
        // all data consumed, reset buffer
        buff.clear();
        last = first = 0;
        buff.reserve(1024);
    }
}

const char* Buffer::data() const { return (last > first) ? (buff.data() + first) : nullptr; }