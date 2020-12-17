#ifndef MRPCAPPLICATION_H
#define MRPCAPPLICATION_H

#include "mrpcConfig.h"
#include "mrpcChannel.h"
#include "mrpcController.h"

//mrpc框架的基础类，负责框架的初始化
class mrpcApplication
{
public:
    static void init(int argc, char **argv);
    static mrpcApplication &getInstance();
    static mrpcConfig &getMrpcConfig();

private:
    static mrpcConfig mConfig;
    mrpcApplication() {}
    mrpcApplication(const mrpcApplication &) = delete;
    mrpcApplication(mrpcApplication &&) = delete;
};

#endif //MRPCAPPLICATION_H