#pragma once

#include<sys/epoll.h>

#include <vector>

#include "Poller.h"

class Timestamp;

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller();
    Timestamp poll(const int& time_out, ChannelList& active_channel) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    void update(const int&, Channel*);
    using EventList = std::vector<epoll_event>;
    static const int kInitSize = 16;
    int epoll_fd;
    // 就绪列表
    EventList list;
};

