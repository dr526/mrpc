#ifndef MRPCLOGGER_H
#define MRPCLOGGER_H

#include "mrpcLogQueue.h"

enum logLevel
{
    INFO, //普通信息
    ERROR //错误信息
};

class mrpcLogger
{
public:
    //获取日志的单例
    static mrpcLogger &getInstance();
    //设置日志级别
    void setLogLevel(logLevel level);
    //把日志信息写入mrpcLogQueue缓冲区中
    void log(std::string msg);

private:
    int logLevel_;                      //记录日志级别
    mrpcLogQueue<std::string> logQueue; //日志缓冲队列

    mrpcLogger();
    mrpcLogger(const mrpcLogger &) = delete;
    mrpcLogger(mrpcLogger &&) = delete;
};

//定义宏, ##__VA_ARGS__代表可变参参数列表
#define LOG_INFO(logMsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        mrpcLogger &logger = mrpcLogger::getInstance();   \
        logger.setLogLevel(INFO);                         \
        char str[1024];                                   \
        snprintf(str, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(str);                                  \
    } while (0);

#define LOG_ERR(logMsgFormat, ...)                        \
    do                                                    \
    {                                                     \
        mrpcLogger &logger = mrpcLogger::getInstance();   \
        logger.setLogLevel(ERROR);                        \
        char str[1024];                                   \
        snprintf(str, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(str);                                  \
    } while (0);

#endif //MRPCLOGGER_H