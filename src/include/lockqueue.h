#pragma once
#include<queue>
#include<mutex>
#include<condition_variable>
#include<thread>


// 异步写日志的日志队列
// 注意的，当队列为空，不能写日志，一定要不为空才能写
template<typename T>
class LockQueue
{
public:
    //  向队列添加元素------>主线程的操作
    void Push(const T & data)
    {
        // 先抢到锁的人先插入元素
        // 或者使用lock_guard
        std::unique_lock<std::mutex>lock(m_mutex);
        m_queue.push(data);
        // 提醒消费者也就是这里的Pop函数消费队列中的数据
        // 由于我们只使用一个线程进行写事件，所以只需要唤醒一个写线程就可以
        m_cond_variable.notify_one();
    }
    T Pop() // -------》后台写日志线程的操作
    {
        // 循环检测队列是否为空
        // 如果队列为空，此时不能消费队列中的元素
        // 所以需要通过条件变量提醒Push往线程中加数据
        // 队列在这里为临界资源

        // 将元素弹出时，必须先获取锁
        std::unique_lock<std::mutex>_lock(m_mutex);
        // 这里的操作用while是防止虚假唤醒
        while(m_queue.empty())
        {
            // 队列为空
            // 不能弹出元素，所以需要释放锁资源
            m_cond_variable.wait(_lock);
        }

        // 队列元素不为空，可以弹出元素以及获取队列的头部元素
        T data = m_queue.front();
        // 弹出元素
        m_queue.pop();
        return data;
    }
    
private:
    std::queue<T>m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond_variable;
};