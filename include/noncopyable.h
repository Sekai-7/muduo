#pragma once

// 作为父类，可以确保继承他的子类不可拷贝
// 为什么构造和析构要设置成protected？子类可以构造对象，不可以构造父类对象
class noncopyable
{
public:
    noncopyable(const noncopyable& nc) = delete;
    noncopyable operator=(const noncopyable& nc) = delete;
protected:
    noncopyable() {}
    ~noncopyable() {}
};

