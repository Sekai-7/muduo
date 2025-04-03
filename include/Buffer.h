#pragma once

#include <vector>
#include <string>

// 总的来说，Buffer的数据是给服务端读的
class Buffer
{
public:
    // 头部预留空间
    static const size_t kCheapPrepend = 8;
    static const size_t kInitSize = 1024;
    explicit Buffer(size_t init_size = kInitSize);
    ~Buffer() = default;

    size_t readableBytes() const;
    size_t writeableBytes() const;
    size_t prependableBytes() const;

    // 返回可读数据的起始地址
    const char* peek() const;

    // 修改read_index的位置
    void retrieve(const size_t& len);
    void retrieveAll();

    // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString();
    std::string retrieveAsString(const size_t& len);

    void ensureWriteableBytes(const size_t& len);

    // 把data的数据添加到writable缓冲区中
    void append(const char* data, size_t len);

    char* beginWrite();
    const char* beginWrite() const;

    // 从fd上读取数据
    ssize_t readFd(const int&, int *);
    // 通过fd发送数据
    ssize_t writeFd(const int&, int *);
private:
    char* begin();
    const char* begin() const;
    void makeSpace(const size_t& len);
    std::vector<char> buffer;
    size_t read_index;
    size_t write_index;
};

