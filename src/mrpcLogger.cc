#include <iostream>
#include <time.h>
#include "mrpcLogger.h"

mrpcLogger::mrpcLogger()
{
    //启动专门的写日志线程，将mrpcLogQueue缓冲区的数据写入log文件中
    std::thread writeLogTask([&]() {
        for (;;)
        {
            //获取当前的日期，然后取日志信息，写入相应的日志文件中
            time_t now = time(nullptr);
            tm *nowTime = localtime(&now);
            char fileName[128];
            sprintf(fileName, "%d-%d-%d-log.txt", nowTime->tm_year + 1900, nowTime->tm_mon + 1, nowTime->tm_mday);
            FILE *fp = fopen(fileName, "a+");
            if (fp == nullptr)
            {
                std::cout << "mrpcLogger file: " << fileName << "open error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg = logQueue.pop();
            char timeBuf[128];
            sprintf(timeBuf, "[%s] %d:%d:%d => ", (logLevel_ == INFO ? "INFO" : "ERROR"), nowTime->tm_hour, nowTime->tm_min, nowTime->tm_sec);
            msg.insert(0, timeBuf);
            msg.append("\n");
            fputs(msg.c_str(), fp);
            fclose(fp);
        }
    });
    //设置分离线程
    writeLogTask.detach();
}

//获取日志的单例
mrpcLogger &mrpcLogger::getInstance()
{
    static mrpcLogger mrpcLogger_;
    return mrpcLogger_;
}

//设置日志级别
void mrpcLogger::setLogLevel(logLevel level)
{
    logLevel_ = level;
}

//把日志信息写入mrpcLogQueue缓冲区中
void mrpcLogger::log(std::string msg)
{
    logQueue.push(msg);
}