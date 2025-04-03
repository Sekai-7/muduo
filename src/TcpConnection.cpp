#include <sys/errno.h>
#include <sys/sendfile.h>
#include <memory>
#include <atomic>
#include <string>

#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Logger.h"

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, const int& sockfd ,const InetAddress& localaddr, const InetAddress& peeraddr)
    :loop(loop), name(name), state(kConnecting), reading(true), socket(new Socket(sockfd)),
    channel(new Channel(loop, sockfd)), local_addr(localaddr), peer_addr(peeraddr), 
    input_buffer(Buffer()), output_buffer(Buffer())
{
    socket->setKeepAlive(true);
    channel->setReadEventCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel->setWriteEventCallback(std::bind(&TcpConnection::handleWrite, this));
    channel->setCloseEventCallback(std::bind(&TcpConnection::handleClose, this));
    channel->setErrorEventCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {

}

EventLoop* TcpConnection::getLoop() {
    return loop;
}

const std::string& TcpConnection::getName() const {
    return name;
}

const InetAddress& TcpConnection::getLocalAddr() const {
    return local_addr;
}

const InetAddress& TcpConnection::getPeerAddr() const {
    return peer_addr;
}

bool TcpConnection::connected() const {
    return state == kConnected;
}

void TcpConnection::send(const std::string& str) {
    if (state == kConnected) {
        if (loop->isInLoopThread()) {
            sendInLoop(str.c_str(), str.size()); 
        }
        else {
            loop->queueInLoop(std::bind(&TcpConnection::sendInLoop, this, str.c_str(), str.size()));
        }
    } 
}

void TcpConnection::sendInLoop(const void* data, const size_t& len) {
    LOG_INFO("SendInLoop\n");
    ssize_t write_bytes = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state == kDisconnecting)
        LOG_ERROR("Connection has been closed");
    if (!channel->isWriteEvent() && output_buffer.readableBytes() == 0) {
        write_bytes = ::write(channel->getFd(), data, len);
        LOG_INFO("Success Send %ld Bytes, str is %s\n", write_bytes, (char*)data);
        if (write_bytes > 0) {
            remaining = len - write_bytes;
            if (remaining == 0 && wri_cb) {
                loop->queueInLoop(std::bind(wri_cb, shared_from_this()));
            }
        } else {
            write_bytes = 0;
            // EWOULDBLOCK标识非阻塞情况下没有数据
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) 
                    fault_error = true;
            }
        }
    }
    if (!fault_error && remaining > 0) {
        size_t old_len = output_buffer.readableBytes();
        if (old_len + remaining >= high_water_mark && old_len < high_water_mark && high_cb) {
            loop->queueInLoop(std::bind(high_cb, shared_from_this(), old_len + remaining));
        }
        output_buffer.append((char*)data + write_bytes, remaining);
        if (!channel->isWriteEvent())
            channel->enableWriting();
    }
}

void TcpConnection::shutdown() {
    if (state == kConnected) {
        setState(kDisconnecting);
        loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    // 在没东西写之后
    if (!channel->isWriteEvent()) {
        socket->shutdownWrite();
    }
}

// 在tcpserver的newconnection被调用
void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel->initTie(shared_from_this());
    channel->enableReading();
    LOG_INFO("New Connection Established\n");
    con_cb(shared_from_this());
}

// 在tcpserver被封装成removeconnection，再传给tcpserver作
// closecallback
void TcpConnection::connectDestroyed() {
    if (state = kConnected) {
        setState(kDisconnected);
        channel->disableReading();
        channel->disableWriting();
        con_cb(shared_from_this());
    }
    channel->remove();
}

// 设置为channel的读回调
void TcpConnection::handleRead(const Timestamp& recv_time) {
    int save_errno = 0;
    ssize_t size = input_buffer.readFd(channel->getFd(), &save_errno);
    if (size > 0) {
        if (mes_cb)
            mes_cb(shared_from_this(), &input_buffer, recv_time);
    } else if (size == 0) {
        handleClose();
    } else {
        LOG_ERROR("handleread Error!\n");
        handleError();
    }
}

// 被调用的时候应该肯定在本线程？
void TcpConnection::handleWrite() {
    if (channel->isWriteEvent()) {
        int saved_errno = 0;
        ssize_t write_bytes = output_buffer.writeFd(channel->getFd(), &saved_errno);
        if (write_bytes > 0) {
            output_buffer.retrieve(write_bytes);
            if (output_buffer.readableBytes() == 0) {
                channel->disableWriting();
                if (wri_cb) {
                    loop->queueInLoop(std::bind(wri_cb, shared_from_this()));
                }
                // 说明调用过shutdown了，但是因为有数据没发送完没完成流程
                if (state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("handleclose()\n");
    setState(kDisconnected);
    // 取消channel监听
    channel->disableReading();
    channel->disableWriting();

    con_cb(shared_from_this());
    cls_cb(shared_from_this());
}

void TcpConnection::handleError() {
        int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel->getFd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name.c_str(), err);
}

void TcpConnection::sendFile(const int& file_fd, off_t& off, const size_t& size) {
    if (connected()) {
        if (loop->isInLoopThread())
            sendFileInLoop(file_fd, off, size);
        else {
            loop->queueInLoop(std::bind(&TcpConnection::sendFileInLoop, this, file_fd, off, size));
        }
    }
}

void TcpConnection::sendFileInLoop(const int& file_fd, off_t& off, const size_t& size) {
    ssize_t write_bytes = 0;
    size_t remaining = size;
    bool fault_error = false;
    if (state == kDisconnecting) {
        LOG_ERROR("Disconnected!\n");
        return;
    }

    if (!channel->isWriteEvent() && output_buffer.readableBytes() == 0) {
        write_bytes = sendfile(channel->getFd(), file_fd, &off, size); 
        if (write_bytes >= 0) {
            remaining -= write_bytes;
            if (remaining == 0 && wri_cb) {
                loop->queueInLoop(std::bind(wri_cb, shared_from_this()));
            }
        } else {
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendFileInLoop\n");
            }
            if (errno == EPIPE || errno == ECONNRESET)
                fault_error = true;
        }
    }

    if (!fault_error && remaining > 0) {
        loop->queueInLoop(std::bind(&TcpConnection::sendFileInLoop, this, file_fd, off, size));
    }
}

void TcpConnection::setState(const State& state) {
    this->state = state;
    return;
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) {
    con_cb = cb;
    return;
}
void TcpConnection::setMessageCallback(const MessageCallback& cb) {
    mes_cb = cb;
    return;
}
void TcpConnection::setWriteCompleteCallback(const WriteCallback& cb) {
    wri_cb = cb;
    return;
}
void TcpConnection::setCloseCallback(const CloseCallback& cb) {
    cls_cb = cb;
} 
void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, const size_t& size) {
    high_cb = cb;
    high_water_mark = size;
    return;
}