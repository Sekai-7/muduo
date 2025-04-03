#include "Poller.h"
#include "EpollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    return new EpollPoller(loop);
}