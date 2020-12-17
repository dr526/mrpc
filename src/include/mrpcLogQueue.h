#ifndef MRPCLOGQUEUE_H
#define MRPCLOGQUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

//异步日志队列
template <typename T>
class mrpcLogQueue
{
public:
    //多个worker线程都会向mrpcLogQueue写入日志
    void push(const T &data)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
        condVariable.notify_one();
    }

    //一个线程从mrpcLogQueue读取日志
    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty())
        {
            //日志队列为空，线程进入wait状态
            condVariable.wait(lock);
        }
        T data = queue_.front();
        queue_.pop();
        return data;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condVariable;
};

#endif //MRPCLOGQUEUE_H
