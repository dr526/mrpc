#include <iostream>
#include "mrpcApplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    //整个程序启动后，通过mrpc框架使用rpc服务，需要先调用框架的初始函数
    mrpcApplication::init(argc, argv);
    //演示调用远程发布的rpc方法login
    dr526::userServiceRpc_Stub stub(new mrpcChannel());
    //rpc方法的请求参数
    dr526::loginRequest request;
    request.set_name("dr526");
    request.set_pwd("123456");
    //rpc方法的响应
    dr526::loginResponse response;
    //定义一个RpcController,用于查看rpc方法是否调用成功
    mrpcController controller;
    //发起rpc方法的调用,同步阻塞的方式
    //mrpcChannel->callMethod(处理所有rpc方法调用的参数序列化和网络发送)
    stub.login(&controller, &request, &response, nullptr);
    //rpc调用是否成功
    if (!controller.Failed())
    {
        //一次rpc调用完成，读取调用的结果
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc login response error： " << response.result().errmsg() << std::endl;
        }
        else
        {
            std::cout << "rpc login response success： " << response.success() << std::endl;
        }
    }
    else
    {
        std::cout << controller.ErrorText() << std::endl;
    }

    //演示调用远程发布的rpc方法register
    dr526::registeRequest registeRequest;
    registeRequest.set_name("dr526");
    registeRequest.set_pwd("123456");
    dr526::registeResponse registeResponse;
    controller.Reset();
    stub.registe(&controller, &registeRequest, &registeResponse, nullptr);
    if (!controller.Failed())
    {
        if (registeResponse.result().errcode() == 0)
        {
            std::cout << "rpc registe response error： " << registeResponse.result().errmsg() << std::endl;
        }
        else
        {
            std::cout << "rpc registe response success： " << registeResponse.success() << std::endl;
        }
    }
    else
    {
        std::cout << controller.ErrorText() << std::endl;
    }

    return 0;
}