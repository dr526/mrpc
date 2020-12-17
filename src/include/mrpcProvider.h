#ifndef MRPCPROVIDER_H
#define MRPCPROVIDER_H
#include <unordered_map>
#include <string>
#include <functional>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

//框架提供的专门负责发布rpc服务的网络对象类
class mrpcProvider
{
public:
    //这是框架提供给外部使用，可以发布rpc方法的函数接口
    void notifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void run();

private:
    //组合EventLoop，相当于epoll
    muduo::net::EventLoop mrpcEventLoop;
    //service服务类型信息
    struct serviceInfo
    {
        //保存服务对象
        google::protobuf::Service *mService;
        //保存服务方法
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> mMethodMap;
    };
    //存储注册成功的服务对象和服务方法的所有信息
    std::unordered_map<std::string, serviceInfo> mServiceMap;
    //新的socket连接回调
    void onConnection(const muduo::net::TcpConnectionPtr &);
    //已建立连接用户的读写事件回调
    void onMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    //Closure回调操作，用于序列化rpc的响应和网络发送
    void sendMrpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};

#endif //MRPCPROVIDER_H