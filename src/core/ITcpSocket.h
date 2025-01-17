#pragma once

#include <cstddef>

class ITcpSocket {
public:
    virtual ~ITcpSocket() = default;
    virtual void write(const char* data, size_t length) = 0;
};
