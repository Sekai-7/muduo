#pragma once

#include <cstdint>
#include <string>

// 有参构造用explicit避免隐式转换
// tostring返回秒数转换得到的格式化时间
// now返回当前距离1970:01:01的秒数（以Timestamp对象的格式）
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(const int64_t&);
    static Timestamp now();
    std::string toString() const;
    ~Timestamp() = default;
private:
    int64_t second_count;
};

