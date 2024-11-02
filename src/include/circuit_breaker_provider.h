#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "circuit_breaker.h"

class CircuitBreakerProvider {
public:
    // 根据服务名称获取或创建熔断器实例
    std::shared_ptr<CircuitBreaker> getCircuitBreaker(const std::string& serviceName);

private:
    std::unordered_map<std::string, std::shared_ptr<CircuitBreaker>> circuitBreakerMap;  // 存储服务名称和熔断器的映射
    mutable std::mutex mutex_;  // 互斥锁，确保线程安全
};
