#include "Logger.h"
#include "Timestamp.h"

#include <string>
#include <iostream>

Logger& Logger::get_instance() {
    static Logger log;
    return log;
}

void Logger::set_level(const int& level) {
    log_level = level;
    return;
} 

void Logger::print_log(const std::string& log_str) {
    std::string level_str;
    switch (log_level)
    {
    case INFO: 
        level_str = "INFO";
        break;
    case ERROR:
        level_str = "ERROR";
        break;
    case FATAL:
        level_str = "FATAL";
        break;
    case DEBUG:
        level_str = "DEBUG";
        break;
    default:
        break;
    }
    // 这边不加endl的话，重定向时可能不会刷新缓冲区（从行缓存到全缓冲）
    std::cout << "[" << Timestamp::now().toString() << "] " << level_str << ": " << log_str << std::endl; 
}
// Test
// int main() {
//     LOG_INFO("this is %d!\n", 23);
//     return 0;
// }