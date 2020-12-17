#include <iostream>
#include <string>
#include "mrpcConfig.h"
#include "mrpcLogger.h"

//负责解析加载配置文件
void mrpcConfig::loadConfigFile(const char *configFile)
{
    FILE *fp = fopen(configFile, "r");
    if (fp == nullptr)
    {
        std::cout << "<configfile> is not exist!" << std::endl;
        LOG_ERR("<configfile> is not exist!");
        exit(EXIT_FAILURE);
    }

    while (!feof(fp))
    {
        char buf[225];
        fgets(buf, 225, fp);

        //去掉字符串前面多余的空格
        std::string srcBuf(buf);
        trim(srcBuf);
        //判断是否为注释
        if (srcBuf[0] == '#' || srcBuf.empty())
            continue;
        //解析配置项
        int idx = srcBuf.find('=');
        //配置项不合法
        if (idx == -1)
            continue;
        std::string key;
        std::string value;
        key = srcBuf.substr(0, idx);
        //去除多余空格
        trim(key);
        int ednIdx = srcBuf.find('\n', idx);
        value = srcBuf.substr(idx + 1, ednIdx - idx - 1);
        trim(value);
        mrpcConfigMap.insert({key, value});
    }
}

//查询配置项信息
std::string mrpcConfig::load(const std::string &key)
{
    auto it = mrpcConfigMap.find(key);
    if (it == mrpcConfigMap.end())
    {
        return "";
    }
    return it->second;
}

//去掉字符串前后的空格
void mrpcConfig::trim(std::string &srcBuf)
{
    int idx = srcBuf.find_first_not_of(' ');
    if (idx != -1)
    {
        //说明字符串前面有空格
        srcBuf = srcBuf.substr(idx, srcBuf.size() - idx);
    }
    //去掉字符串后面多余的空格
    idx = srcBuf.find_last_not_of(' ');
    if (idx != -1)
    {
        //说明字符串后面有空格
        srcBuf = srcBuf.substr(0, idx + 1);
    }
}