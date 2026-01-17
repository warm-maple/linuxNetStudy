// Buffer.cpp - implementations
#include "../include/Buffer.h"
#include <cstring>
#include <sys/socket.h>
#include <errno.h>

Buffer::Buffer() { buff.reserve(8192); }

size_t Buffer::size() const { return buff.size(); }

void Buffer::buffclear() { buff.clear(); }

std::string Buffer::getMes() const { return std::string(buff.begin(), buff.end()); }

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
            buff.insert(buff.end(), temp_buf, temp_buf + read_len);
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
    return static_cast<ssize_t>(buff.size());
}

const char* Buffer::data() const { return buff.empty() ? nullptr : buff.data(); }