#pragma once
#include<string>
#include "stdio.h"

#include "noncopyable.h"

class Logger;

enum level {
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core dump信息
    DEBUG, // 调试信息
};

// 调用形式就类似于printf那样，格式化的字符串
// __VA_ARGS__代表可变参数
// 加两个##是为了防止没有格式化字符串，也就没有可变参数
// 为什么用dowhile？dowhile是一句语句，可以防止出现问题（比如if后面没有加括号，但是写了一个日志输出）
// snprintf是把后面的内容写到buffer里，传入的size是如果大于这个size会截断
// 注意log要写成引用，不如会触发拷贝构造，但是noncopyable已经delete拷贝构造了
#define LOG_INFO(message, ...)  \
    do {                        \
        Logger& log = Logger::get_instance();\
        log.set_level(INFO);\
        char buffer[1024] = {0};\
        snprintf(buffer, 1024, message, ##__VA_ARGS__);\
        log.print_log(buffer);\
    }while(0)                   \

#define LOG_ERROR(message, ...)  \
    do {                        \
        Logger& log = Logger::get_instance();\
        log.set_level(ERROR);\
        char buffer[1024] = {0};\
        snprintf(buffer, 1024, message, ##__VA_ARGS__);\
        log.print_log(buffer);\
    }while(0)                   \

#define LOG_FATAL(message, ...)  \
    do {                        \
        Logger& log = Logger::get_instance();\
        log.set_level(FATAL);\
        char buffer[1024] = {0};\
        snprintf(buffer, 1024, message, ##__VA_ARGS__);\
        log.print_log(buffer);\
        exit(-1);\
    }while(0)                   \

#define LOG_DEBUG(message, ...)  \
    do {                        \
        Logger& log = Logger::get_instance();\
        log.set_level(DEBUG);\
        char buffer[1024] = {0};\
        snprintf(buffer, 1024, message, ##__VA_ARGS__);\
        log.print_log(buffer);\
    }while(0)                   \

class Logger : noncopyable 
{
public:
    static Logger& get_instance();
    void set_level(const int&); 
    void print_log(const std::string&);
private:
    Logger() = default;
    int log_level;
};

