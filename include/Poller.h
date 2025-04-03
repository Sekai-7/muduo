#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"


class EventLoop;
class Channel;
class Timestamp;

// poller是用来监听文件描述符的
// 当文件描述符有被监听的事件发生，其就会被存储在
class Poller : noncopyable {
public:
    // 被唤醒的fd
    using ChannelList = std::vector<Channel*>;
    // fd和Channel的映射
    // 这个映射在后续没有功能上的作用？（可能仅作提供）
    using ChannelMap = std::unordered_map<int, Channel*>;
    Poller(EventLoop*);
    virtual ~Poller() = default;
    // 这个activeChannel到时候会存epollwait返回的已就绪的文件描述符
    // 其定义在eventloop中
    // 注意整个架构poll和channel实际都为eventloop服务
    virtual Timestamp poll(const int& time_out, ChannelList& active_channel) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    // 判断channel是否由这个poll监听
    bool hasChannel(Channel* channel) const;
    // 获取一个poll实例
    // 这个函数不会在Poller.cc中实现
    // 因为这个函数需要返回一个poller的实例，显然poller类无法实例化
    // 所以返回的是poller类的子类实例
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    // 这边为什么要protected呢，
    // 感觉可能是子类需要使用，但是外部不让访问
    ChannelMap channel_map;
private:
    EventLoop* loop;
};

