#pragma once

#include <memory>
#include <atomic>
#include <string>

#include "noncopyable.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"

class Socket;
class EventLoop;
class Channel;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop*, const std::string&, const int&, const InetAddress&, const InetAddress&);
    ~TcpConnection();

    EventLoop* getLoop();
    const std::string& getName() const;
    const InetAddress& getLocalAddr() const;
    const InetAddress& getPeerAddr() const;

    bool connected() const;    

    // 发送数据
    void send(const std::string&);
    void sendFile(const int&, off_t&, const size_t&);

    // 关闭半连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCallback&);
    void setCloseCallback(const CloseCallback&);
    void setHighWaterMarkCallback(const HighWaterMarkCallback&, const size_t&);

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();
private:
    enum State {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void setState(const State&);

    void handleRead(const Timestamp&);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void*, const size_t&);
    void shutdownInLoop();
    void sendFileInLoop(const int&, off_t&, const size_t&);

    EventLoop* loop;
    const std::string name;

    std::atomic<int> state;

    bool reading;

    std::unique_ptr<Socket> socket;
    std::unique_ptr<Channel> channel;

    // 本地ip和端口
    const InetAddress local_addr;
    // 远程ip和端口
    const InetAddress peer_addr;

    // 回调函数，用户设置给tcpserver，tcpserver再设置给tcpconnection
    ConnectionCallback con_cb;
    MessageCallback mes_cb;
    WriteCallback wri_cb;
    HighWaterMarkCallback high_cb;
    CloseCallback cls_cb;

    size_t high_water_mark;


    // 数据缓冲区
    Buffer input_buffer;
    Buffer output_buffer;
};

