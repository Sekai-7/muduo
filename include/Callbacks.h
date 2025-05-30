#pragma once

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

using WriteCallback = std::function<void(const TcpConnectionPtr&)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, const size_t&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, const Timestamp&)>;