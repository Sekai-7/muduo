#include <vector>
#include <string>
#include <unistd.h>
#include <sys/uio.h>

#include "Buffer.h"
#include "Logger.h"

Buffer::Buffer(size_t init_size):buffer(init_size + kCheapPrepend), read_index(0), write_index(0)
{}

size_t Buffer::readableBytes() const {
    return write_index - read_index;
}

size_t Buffer::writeableBytes() const {
    return buffer.size() - write_index;
}

size_t Buffer::prependableBytes() const {
    return read_index;
}

const char* Buffer::peek() const {
    return begin() + read_index;
}

void Buffer::retrieve(const size_t& len) {
    if (len <= readableBytes())
        read_index += len;
    else 
        retrieveAll();
}

void Buffer::retrieveAll() {
    read_index = write_index = kCheapPrepend;
    return;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(const size_t& len) {
    std::string temp = std::string(peek(), len);
    retrieve(len);
    return temp; 
}

void Buffer::ensureWriteableBytes(const size_t& len) {
    if (len <= writeableBytes())
        return;
    makeSpace(len);
    return;
}

void Buffer::append(const char* data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    return;
}

char* Buffer::beginWrite() {
    return begin() + write_index;
}

const char* Buffer::beginWrite() const {
    return begin() + write_index;
}

ssize_t Buffer::readFd(const int& fd, int* save_error) {
    char extra_buffer[65536] = {0};
    iovec vec[2];
    vec[0].iov_base = begin() + write_index;
    vec[0].iov_len = writeableBytes();
    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof(extra_buffer);
    int iovcnt = vec[0].iov_len < vec[1].iov_len ? 2 : 1;
    ssize_t ret = ::readv(fd, vec, iovcnt);
    if (ret < 0)
        save_error = &errno;
    else if (ret <= writeableBytes())
        write_index += ret;
    else {
        int size = writeableBytes();
        write_index = buffer.size();
        append(extra_buffer, ret - size);
    }
    std::string temp(peek(), ret);
    LOG_INFO("Success Read %ld Bytes, str is %s\n", ret, temp.c_str());
    return ret;
}

ssize_t Buffer::writeFd(const int& fd, int* save_error) {
    size_t size = ::write(fd, peek(), readableBytes());
    if (size < 0)
        *save_error = errno;
    return size;
}

char* Buffer::begin() {
    return &*buffer.begin();
}

const char* Buffer::begin() const {
    return &*buffer.begin();
}

void Buffer::makeSpace(const size_t& len) {
    if (writeableBytes() + read_index - kCheapPrepend < len) {
        buffer.resize(len + write_index);
    } else {
        int size = readableBytes();
        std::copy(begin() + read_index, begin() + write_index, begin() + kCheapPrepend);
        read_index = kCheapPrepend;
        write_index = read_index + size;
    }
    return;
}