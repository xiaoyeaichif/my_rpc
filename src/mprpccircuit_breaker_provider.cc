#include "circuit_breaker_provider.h"


// 根据服务名称获取或创建熔断器实例
std::shared_ptr<CircuitBreaker> CircuitBreakerProvider::getCircuitBreaker(const std::string& serviceName) {
    std::lock_guard<std::mutex> lock(mutex_);  // 加锁，确保线程安全
    auto it = circuitBreakerMap.find(serviceName);  // 查找服务名称对应的熔断器
    if (it != circuitBreakerMap.end()) {  // 如果存在
        return it->second;  // 返回已存在的熔断器
    } else {
        // 如果不存在，创建一个新的熔断器并存储
        // 失败次数超过3次开启熔断器
        // 超过50%的接受率，从半开状态中恢复为关闭状态
        // 默认是10s时间,熔断器尝试从打开转到半打开状态
        auto circuitBreaker = std::make_shared<CircuitBreaker>(3, 0.5, 10000);
        circuitBreakerMap[serviceName] = circuitBreaker;
        return circuitBreaker;  // 返回新创建的熔断器
    }
}