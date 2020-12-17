#include <iostream>
#include <unistd.h>
#include <string>
#include "mrpcApplication.h"
#include "mrpcLogger.h"

mrpcConfig mrpcApplication::mConfig;

void showArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
    LOG_ERR("format: command -i <configfile>");
}

//provider -i config.conf
void mrpcApplication::init(int argc, char **argv)
{
    if (argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string configFile;

    //getopt函数用于解析命令行，将-i后的参数使用optarg指向
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            configFile = optarg;
            break;
        //出现不在规定范围内的参数，例如：-a等
        case '?':
            showArgsHelp();
            exit(EXIT_FAILURE);
        //出现指定的参数-i，但是后面没有参数
        case ':':
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    //开始加载配置文件
    mConfig.loadConfigFile(configFile.c_str());
    std::cout << "rpcServerIp：" << mConfig.load("rpcServerIp") << std::endl;
    std::cout << "rpcServerPort：" << mConfig.load("rpcServerPort") << std::endl;
    std::cout << "zookeeperIp：" << mConfig.load("zookeeperIp") << std::endl;
    std::cout << "zookeeperPort：" << mConfig.load("zookeeperPort") << std::endl;
    LOG_INFO("rpcServerIp： %s", mConfig.load("rpcServerIp").c_str());
    LOG_INFO("rpcServerPort： %s", mConfig.load("rpcServerPort").c_str());
    LOG_INFO("zookeeperIp： %s", mConfig.load("zookeeperIp").c_str());
    LOG_INFO("zookeeperPort： %s", mConfig.load("zookeeperPort").c_str());
}

mrpcApplication &mrpcApplication::getInstance()
{
    static mrpcApplication mrpcApp;
    return mrpcApp;
}

mrpcConfig &mrpcApplication::getMrpcConfig()
{
    return mConfig;
}