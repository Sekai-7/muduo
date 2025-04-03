
#include<sys/epoll.h>
#include<sys/unistd.h>

#include<vector>
#include<unordered_map>
#include<ctime>
#include<cstring>

#include "EpollPoller.h"
#include "EventLoop.h"
#include"Channel.h"
#include"Timestamp.h"
#include"Logger.h"

// 这里为什么是三个状态不是两个呢？
// 未解决

const int kNew = -1;    // 某个channel还没添加至Poller          // channel的成员index_初始化为-1
const int kAdded = 1;   // 某个channel已经添加至Poller
const int kDeleted = 2; // 某个channel已经从Poller删除

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop), epoll_fd(epoll_create(10)), list(kInitSize)
    {} 

EpollPoller::~EpollPoller() {
    close(epoll_fd);
}

Timestamp EpollPoller::poll(const int& time_out, ChannelList& active_channel) {
    int epoll_count = epoll_wait(epoll_fd, list.data(), list.size(), time_out);
    if (epoll_count == list.size()) {
        list.resize(2 * list.size());
    }
    if (epoll_count == 0) {
        LOG_INFO("Time_Out!\n");
        return Timestamp(Timestamp::now());
    } else {
        LOG_INFO("epoll_fd is %d get %d events\n", epoll_fd, epoll_count);
    }
    // 这边不能用reserve
    active_channel.resize(epoll_count);
    for (int i = 0; i < epoll_count; ++i) {
        int fd = list[i].data.fd;
        Channel* channel = channel_map[fd];
        channel->setReal_events(list[i].events);
        active_channel[i] = channel;
         // LOG_INFO("epoll_fd save %d\n", i);
    }
    // list.resize(0);
    return Timestamp(Timestamp::now());
}

void EpollPoller::updateChannel(Channel* channel) {
    int index = channel->getIndex();
    if (index == kNew || index == kDeleted) {
        channel_map[channel->getFd()] = channel;
        LOG_INFO("Poller::updataChannel %d Add\n", epoll_fd);
        update(EPOLL_CTL_ADD, channel);
    }
    else if (index == kAdded && channel->isNoneEvent()) {
        channel->setIndex(kDeleted);
        update(EPOLL_CTL_DEL, channel);
        LOG_INFO("Poller::updataChannel %d DEL\n", epoll_fd);
        return;
    } else {
        update(EPOLL_CTL_MOD, channel);
        LOG_INFO("Poller::updataChannel %d MOD\n", epoll_fd);
    }
    channel->setIndex(kAdded);
    return;
}

void EpollPoller::removeChannel(Channel* channel) {
    if (channel->getIndex() == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
    channel_map.erase(channel->getFd());
    return;
}

void EpollPoller::update(const int& operation, Channel* channel) {
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = channel->getFd();
    event.events = channel->getEvents();
    if (event.events | EPOLLIN)
        LOG_INFO("Add read!\n");
    epoll_ctl(epoll_fd, operation, channel->getFd(), &event);
    return;
}