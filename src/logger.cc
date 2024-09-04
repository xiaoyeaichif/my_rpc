#include "logger.h"
#include <time.h>
#include <iostream>


 // 获取日志的单例，线程安全的懒汉式模式
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
// 设置日志级别 
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}
// 写日志， 把日志信息写入lockqueue缓冲区当中
void Logger::Log(std::string &msg)
{
    m_lckQue.Push(msg);
}

// 默认构造函数的实现
Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask(
        // [&] 捕获全部变量以引用的方式
        // 形参列表为空
        // 返回值为void
        [&]()
        {
            // 后台线程一直将日志写进队列中
            while(true)
            {
                // 获取当前的日期
                time_t now = time(nullptr);
                // 具体的时分秒都在tm这个结构体中
                tm *nowtm = localtime(&now);

                char file_name[128]; // 缓冲区大小
                // 格式化字符串
                // 只有具体的年月日
                sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

                FILE *pf = fopen(file_name, "a+");
                if (pf == nullptr)
                {
                    std::cout << "logger file : " << file_name << " open error!" << std::endl;
                    exit(EXIT_FAILURE);
                }

                // 获取队头的日志信息
                std::string msg = m_lckQue.Pop();

                // 给年月日加上时分秒
                // 并且判断日志的等级
                char time_buf[128] = {0};
                sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                        nowtm->tm_hour, 
                        nowtm->tm_min, 
                        nowtm->tm_sec,
                        (m_loglevel == INFO ? "info" : "error"));
                msg.insert(0, time_buf);
                msg.append("\n");

                // 将组合时分秒的数据日志放在日志文件的一行中
                fputs(msg.c_str(), pf);
                // 关闭文件
                fclose(pf);
            }
        }
    );
    // 与主线程分离,后台线程一直执行
    writeLogTask.detach();
}