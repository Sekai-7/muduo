#include <sys/epoll.h>

#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* el, const int&fd)
:loop(el), fd(fd), events(0), 
real_events(0), index(-1), tied(false) {}

void Channel::update() {
    loop->updateChannel(this);
}

void Channel::remove() {
    loop->removeChannel(this);
}

void Channel::setReadEventCallback(ReadEventCallback rec) {
    read_callback = rec;
}

void Channel::setWriteEventCallback(EventCallback ec) {
    write_callback = ec;
}

void Channel::setCloseEventCallback(EventCallback ec) {
    close_callback = ec;
}

void Channel::setErrorEventCallback(EventCallback ec) {
    error_callback = ec;
}

void Channel::enableReading() {
    events = events | kReadEvent;
    LOG_INFO("enableReading\n");
    update();
}

void Channel::disableReading() {
    events = events & ~kReadEvent;
    update();
}

void Channel::enableWriting() {
    events = events | kWriteEvent;
    update();
}

void Channel::disableWriting() {
    events = events & ~kWriteEvent;
    update();
}

bool Channel::isNoneEvent() const {
    return events & kNoneEvent;
}

bool Channel::isReadEvent() const {
    return events & kReadEvent;
}

bool Channel::isWriteEvent() const {
    return events & kWriteEvent;
}

EventLoop* Channel::owernLoop() {
    return loop;
}

int Channel::getIndex() const {
    return index;
}

void Channel::setIndex(const int& idx) {
    index = idx;
    return;
}

int Channel::getEvents() const {
    return events;
}

void Channel::setReal_events(const int& e) {
    real_events = e;
}

int Channel::getFd() const {
    return fd;
}

void Channel::initTie(const std::shared_ptr<void>& obj) {
    tie = obj;
    tied = true;
}

void Channel::handleEvent(const Timestamp& recv_time) {
    // LOG_INFO("handlEvent\n");
    if (tied) {
        std::shared_ptr<void> temp = tie.lock();
        if (temp) {
            handleEventGuard(recv_time);
        }
    // 这个实际是留给监听fd执行的，因为监听的channel不会设置tie
    } else {
        handleEventGuard(recv_time);
    }
}

void Channel::handleEventGuard(const Timestamp& rec_time) {
    // 这边很多情况没考虑
    if ((real_events & EPOLLHUP) && !(real_events & EPOLLIN)) {
        LOG_INFO("handle_close\n");
        if (close_callback)
            close_callback();
    }
    else if (real_events & EPOLLERR) {
        LOG_INFO("handle_error\n");
        if (error_callback)
            error_callback();
    }
    else if (real_events & (EPOLLIN | EPOLLPRI)) {
        LOG_INFO("handle_read\n");
        if (read_callback)
            read_callback(rec_time);
    }
    else if (real_events & EPOLLOUT) {
        LOG_INFO("handle_write\n");
        if (write_callback)
            write_callback();
    }
    return;
}
