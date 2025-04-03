#pragma once
#include <memory>
#include <functional>

class EventLoop;
class Timestamp;

class Channel
{
    using ReadEventCallback = std::function<void(Timestamp)>;
    using EventCallback = std::function<void()>;
public:
    Channel(EventLoop*, const int&);
    ~Channel() = default;

    // 收到通知后，处理事件
    void handleEvent(const Timestamp&);

    void initTie(const std::shared_ptr<void> &);

    int getFd() const;
    int getEvents() const; 
    void setReal_events(const int&);

    // 设置回调函数
    void setReadEventCallback(ReadEventCallback);
    void setWriteEventCallback(EventCallback);
    void setCloseEventCallback(EventCallback);
    void setErrorEventCallback(EventCallback);

    // 修改当前事件监听状态
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();

    // 返回当前事件监听状态
    bool isNoneEvent() const;
    bool isReadEvent() const;
    bool isWriteEvent() const;

    // 移除当前channel
    void remove();

    // 返回当前channel所属的循环
    EventLoop* owernLoop();

    int getIndex() const;
    void setIndex(const int& idx);

private:

    // 更新当前监听状态到poll
    void update();

    void handleEventGuard(const Timestamp&);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    // 事件循环
    EventLoop* loop;
    // 文件描述符
    const int fd;
    // 监听事件
    int events;
    // 具体发生的事件
    int real_events;
    int index;

    std::weak_ptr<void> tie;
    bool tied;

    // 各个事件的回调函数
    ReadEventCallback read_callback;
    EventCallback write_callback;
    EventCallback close_callback;
    EventCallback error_callback;
};

