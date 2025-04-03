#pragma once

#include<vector>
#include<atomic>
#include<mutex>
#include<memory>
#include<functional>

#include"Timestamp.h"
#include"noncopyable.h"
#include"CurrentThread.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Func = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    Timestamp pollReturnTime() const;
    void runInLoop(Func cb);
    void queueInLoop(Func cb);
    void wakeup();
    // EventLoop的方法 => Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    // 判断EventLoop对象是否在自己的线程里
    // threadId_为EventLoop创建时的线程id CurrentThread::tid()为当前线程id
    bool isInLoopThread() const;
private:
    // 给evnetfd绑定的回调
    void handleRead();
    // 执行上层回调
    void doFunction();
private:
    // 返回epoll监听到触发事件的channel
    using ChannelList = std::vector<Channel*>;
    ChannelList active_channel;
    // 用于loop循环，为什么要两个？
    // 感觉不影响功能也
    std::atomic<bool> looping;
    std::atomic<bool> quiting;
    // eventloop所属的poll
    std::unique_ptr<Poller> poller;
    // poller返回的时间
    Timestamp poll_return_time;
    // eventfd
    int wake_fd;
    // 封装wakefd的channel
    std::unique_ptr<Channel> wake_channel;
    // 当前线程的id
    const pid_t thread_id;
    // 标识是否有回调函数需要执行
    std::atomic<bool> calling_pending_fun;
    std::vector<Func> pending_fun;
    // 访问回调函数组的锁
    std::mutex mutex;
};

