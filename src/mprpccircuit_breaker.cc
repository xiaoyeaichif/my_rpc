#include "circuit_breaker.h"

// 构造函数，初始化熔断器的状态、阈值和时间参数
CircuitBreaker::CircuitBreaker(int failureThreshold, double halfOpenSuccessRate, long retryTimePeriod)
    : state(CircuitBreakerState::CLOSED),  // 初始状态为关闭
      failureCount(0),  // 失败计数初始化为0
      successCount(0),  // 成功计数初始化为0
      requestCount(0),  // 请求计数初始化为0
      failureThreshold(failureThreshold),  // 失败阈值
      halfOpenSuccessRate(halfOpenSuccessRate),  // 半打开状态下的成功率阈值
      retryTimePeriod(retryTimePeriod),  // 重试时间间隔
      lastFailureTime(0) {}  // 最后一次失败的时间初始化为0

// 判断是否允许请求
bool CircuitBreaker::allowRequest() {
    std::lock_guard<std::mutex> lock(mutex_);  // 加锁，确保线程安全
    long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();  // 获取当前时间

    switch (state) {
        case CircuitBreakerState::OPEN:  // 如果状态为打开
            if (currentTime - lastFailureTime > retryTimePeriod) {  // 检查是否超过重试时间
                state = CircuitBreakerState::HALF_OPEN;  // 切换到半打开状态
                resetCounts();  // 重置计数
                return true;  // 允许请求
            }
            return false;  // 不允许请求

        case CircuitBreakerState::HALF_OPEN:  // 如果状态为半打开
            requestCount++;  // 增加请求计数
            return true;  // 允许请求

        case CircuitBreakerState::CLOSED:  // 如果状态为关闭
        default:  // 默认情况下也允许请求
            return true;  // 允许请求
    }
}

// 记录成功请求
void CircuitBreaker::recordSuccess() {
    std::lock_guard<std::mutex> lock(mutex_);  // 加锁，确保线程安全
    if (state == CircuitBreakerState::HALF_OPEN) {  // 如果状态为半打开
        successCount++;  // 增加成功计数
        if (successCount >= halfOpenSuccessRate * requestCount) {  // 检查成功率达到阈值
            state = CircuitBreakerState::CLOSED;  // 切换到关闭状态
            resetCounts();  // 重置计数
        }
    } else {
        resetCounts();  // 重置计数
    }
}

// 记录失败请求
void CircuitBreaker::recordFailure() {
    std::lock_guard<std::mutex> lock(mutex_);  // 加锁，确保线程安全
    failureCount++;  // 增加失败计数
    lastFailureTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();  // 更新最后一次失败时间

    if (state == CircuitBreakerState::HALF_OPEN) {  // 如果状态为半打开
        state = CircuitBreakerState::OPEN;  // 切换到打开状态
    } else if (failureCount >= failureThreshold) {  // 如果失败次数达到阈值
        state = CircuitBreakerState::OPEN;  // 切换到打开状态
    }
}

// 获取当前状态
CircuitBreakerState CircuitBreaker::getState() const {
    return state;  // 返回当前状态
}

// 重置所有计数器
void CircuitBreaker::resetCounts() {
    failureCount = 0;  // 重置失败计数
    successCount = 0;  // 重置成功计数
    requestCount = 0;  // 重置请求计数
}