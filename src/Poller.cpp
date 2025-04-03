#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"

Poller::Poller(EventLoop* loop):loop(loop) {}

bool Poller::hasChannel(Channel* channel) const{
    auto it = channel_map.find(channel->getFd());
    return it != channel_map.end() && it->second == channel;
} 