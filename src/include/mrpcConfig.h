#ifndef MRPCCONFIG_H
#define MRPCCONFIG_H

#include <unordered_map>
#include <string>

//框架读取配置文件类
class mrpcConfig
{
public:
    //负责解析加载配置文件
    void loadConfigFile(const char *configFile);
    //查询配置项信息
    std::string load(const std::string &key);

private:
    //存储rpcServerIp、rpcServerPort、zookeeperIp、zookeeperPort
    //框架只需要初始化一次，可以不考虑线程安全
    std::unordered_map<std::string, std::string> mrpcConfigMap;
    //去掉字符串前后的空格
    void trim(std::string &srcBuf);
};

#endif //MRPCCONFIG_H