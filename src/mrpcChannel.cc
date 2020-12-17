#include <string>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "mrpcHeader.pb.h"
#include "mrpcApplication.h"
#include "mrpcLogger.h"
#include "zookeeperUtil.h"

//所有通过stub代理对象调用的rpc方法，都需要在这里做rpc方法调用的数据序列化和网络发送
//发送给服务提供方的数据格式如下：
//size(4个字节)+serviceName+methodName+argSize[rpc方法参数的序列化字符串长度]+argStr[rpc方法参数]
void mrpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                             google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    //serviceName
    std::string serviceName = sd->name();
    //methodName
    std::string methodName = method->name();
    //argSize[rpc方法参数的序列化字符串长度]
    uint32_t argSize = 0;
    std::string argStr;
    if (request->SerializeToString(&argStr))
    {
        argSize = argStr.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        LOG_ERR("serialize request error!");
        return;
    }
    //定义rpc的请求序列化字符串
    mrpc::rpcHeader rpcHeader_;
    rpcHeader_.set_servicename(serviceName);
    rpcHeader_.set_methodname(methodName);
    rpcHeader_.set_argsize(argSize);
    //调用rpc方法的请求字符串
    std::string requestStr;
    //serviceName+methodName+argSize的长度
    uint32_t size = 0;
    if (rpcHeader_.SerializeToString(&requestStr))
    {
        size = requestStr.size();
    }
    else
    {
        controller->SetFailed("serialize requestStr error!!");
        LOG_ERR("serialize requestStr error!!");
        return;
    }
    //组织待发送的rpc请求序列化字符串
    std::string sendStr;
    //size
    sendStr.insert(0, std::string((char *)&size, 4));
    //serviceName+methodName+argSize
    sendStr += requestStr;
    //argStr
    sendStr += argStr;
    //打印调试信息
    std::cout << "***********************************" << std::endl;
    std::cout << "size： " << size << std::endl;
    std::cout << "serviceName：" << serviceName << std::endl;
    std::cout << "methodName：" << methodName << std::endl;
    std::cout << "argSize：" << argSize << std::endl;
    std::cout << "argStr：" << argStr << std::endl;
    std::cout << "***********************************" << std::endl;
    LOG_INFO("size: %d", size);
    LOG_INFO("serviceName: %s", serviceName.c_str());
    LOG_INFO("methodName: %s", methodName.c_str());
    LOG_INFO("argSize: %d", argSize);
    LOG_INFO("argStr: %s", argStr.c_str());
    //使用tcp编程，完成rpc方法的远程调用
    //创建socket,采用AF_INET地址家族
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1)
    {
        controller->SetFailed("create socket error! errno: " + errno);
        LOG_ERR("create socket error! errno: %d", errno);
        return;
    }
    //读取配置文件的ip和port信息
    // std::string ip = mrpcApplication::getInstance().getMrpcConfig().load("rpcServerIp");
    // uint16_t port = atoi(mrpcApplication::getInstance().getMrpcConfig().load("rpcServerPort").c_str());

    //查询zookeeper节点存储的对应rpc方法所在的ip和port
    zkClient zkClient_;
    zkClient_.start();
    std::string methodPath = "/" + serviceName + "/" + methodName;
    std::string hostData = zkClient_.getData(methodPath.c_str());
    if (hostData == "")
    {
        controller->SetFailed(methodPath + " is not exist!");
        LOG_ERR("%s is not exist", methodPath.c_str());
        return;
    }
    int idx = hostData.find(":");
    if (idx == -1)
    {
        controller->SetFailed(methodPath + " address is invalid!");
        LOG_ERR("%s address is invalid!", methodPath.c_str());
        return;
    }
    std::string ip = hostData.substr(0, idx);
    uint16_t port = atoi(hostData.substr(idx + 1, hostData.size() - idx).c_str());
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    clientAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    //发起连接,连接rpc服务节点
    if (connect(clientFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1)
    {
        close(clientFd);
        controller->SetFailed("connect error! errno: " + errno);
        LOG_ERR("connect error! errno: %d", errno);
        return;
    }
    //发送rpc方法调用请求参数
    if (send(clientFd, sendStr.c_str(), sendStr.size(), 0) == -1)
    {
        close(clientFd);
        controller->SetFailed("send error! errno: " + errno);
        LOG_ERR("send error! errno: %d", errno);
        return;
    }
    //接收rpc请求的响应值
    char recvBuf[1024] = {0};
    uint32_t recvSize = 0;
    if ((recvSize = recv(clientFd, recvBuf, 1024, 0)) == -1)
    {
        close(clientFd);
        controller->SetFailed("receive error! errno: " + errno);
        LOG_ERR("receive error! errno: %d", errno);
        return;
    }
    //responseStr遇到\0导致后面的数据无法存储
    //std::string responseStr(recvBuf, 0, recvSize);
    //将响应数据进行反序列化
    if (!response->ParseFromArray(recvBuf, recvSize))
    {
        close(clientFd);
        controller->SetFailed("parse error! responseStr: " + std::string(recvBuf));
        LOG_ERR("parse error! responseStr: %s", std::string(recvBuf).c_str());
        return;
    }
    close(clientFd);
}