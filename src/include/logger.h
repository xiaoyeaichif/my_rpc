#pragma once
#include "lockqueue.h"
#include <string>


// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
// 可变参的日志宏
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \



// 框架的日志系统

// 日志级别
enum LogLevel
{
    INFO,// 普通信息
    ERROR, // 错误信息
};



// 使用单例模式
class Logger
{
public:
    // 获取日志的单例
    static Logger& GetInstance();
    // 设置日志级别 
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string &msg);
private:
    // 设置日志的级别
    int m_loglevel;
    LockQueue<std::string>m_lckQue; // 日志缓冲队列
    // 
    Logger();
    // 禁止拷贝构造函数，移动构造函数以及赋值函数
    Logger(const Logger &) = delete;
    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger&&) = delete;
};