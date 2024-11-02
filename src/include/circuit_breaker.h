#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

// 定义熔断器的状态枚举
enum class CircuitBreakerState {
    CLOSED,  // 关闭状态
    OPEN,    // 打开状态
    HALF_OPEN // 半打开状态
};

class CircuitBreaker {
public:
    // 构造函数，初始化熔断器的状态、阈值和时间参数
    CircuitBreaker(int failureThreshold, double halfOpenSuccessRate, long retryTimePeriod);

    // 判断是否允许请求
    bool allowRequest();

    // 记录成功请求
    void recordSuccess();

    // 记录失败请求
    void recordFailure();

    // 获取当前状态
    CircuitBreakerState getState() const;

private:
    // 重置所有计数器
    void resetCounts();

    CircuitBreakerState state;  // 当前状态
    std::atomic<int> failureCount;  // 失败计数
    std::atomic<int> successCount;  // 成功计数
    std::atomic<int> requestCount;  // 请求计数
    const int failureThreshold;  // 失败阈值
    const double halfOpenSuccessRate;  // 半打开状态下的成功率阈值
    const long retryTimePeriod;  // 重试时间间隔
    long lastFailureTime;  // 最后一次失败的时间

    mutable std::mutex mutex_;  // 互斥锁，确保线程安全
};