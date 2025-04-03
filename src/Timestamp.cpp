#include "Timestamp.h"

#include <ctime>

Timestamp::Timestamp():second_count(0) {}

Timestamp::Timestamp(const int64_t& time):second_count(time) {}

Timestamp Timestamp::now() {
    return Timestamp(time(nullptr));
}

std::string Timestamp::toString() const {
    char buffer[128] = {0};
    tm* local_time = localtime(&second_count);
    snprintf(buffer, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             local_time->tm_year + 1900,
             local_time->tm_mon + 1,
             local_time->tm_mday,
             local_time->tm_hour,
             local_time->tm_min,
             local_time->tm_sec);
    return buffer;
}