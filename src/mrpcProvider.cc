#include "mrpcProvider.h"
#include "mrpcApplication.h"
#include "mrpcHeader.pb.h"
#include "mrpcLogger.h"
#include "zookeeperUtil.h"

//这是框架提供给外部使用，可以发布rpc方法的函数接口
void mrpcProvider::notifyService(google::protobuf::Service *service)
{
    serviceInfo serviceInfo_;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *serviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string serviceName = serviceDesc->name();
    //获取服务对象方法的数量
    int methodCnt = serviceDesc->method_count();
    std::cout << "serviceName：" << serviceName << std::endl;
    LOG_INFO("serviceName: %s", serviceName.c_str());
    for (int i = 0; i < methodCnt; ++i)
    {
        //获取服务对象指定下标的服务方法描述（抽象描述）
        const google::protobuf::MethodDescriptor *methodDesc = serviceDesc->method(i);
        std::string methodName = methodDesc->name();
        serviceInfo_.mMethodMap.insert({methodName, methodDesc});
        std::cout << "methodName：" << methodName << std::endl;
        LOG_INFO("methodName: %s", methodName.c_str());
    }
    serviceInfo_.mService = service;
    mServiceMap.insert({serviceName, serviceInfo_});
}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void mrpcProvider::run()
{
    std::string ip = mrpcApplication::getInstance().getMrpcConfig().load("rpcServerIp");
    uint16_t port = atoi(mrpcApplication::getInstance().getMrpcConfig().load("rpcServerPort").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&mrpcEventLoop, address, "rpcProvider");
    //绑定连接回调和消息读写回调方法，分离网络代码和业务代码
    server.setConnectionCallback(std::bind(&mrpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&mrpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zookeeper上面，让rpc client可以在zookeeper上发现服务
    //session timout为30s， zkClient网络I/O线程会在1/3*timeout的时间发送心跳信息
    zkClient zkClient_;
    zkClient_.start();
    //serviceName为永久节点，methodName为临时节点
    for (auto &sp : mServiceMap)
    {
        //serviceName
        std::string servicePath = "/" + sp.first;
        zkClient_.create(servicePath.c_str(), nullptr, 0);
        for (auto &mp : sp.second.mMethodMap)
        {
            //路径为/serviceName/methodName，存储数据为当前这个rpc服务节点主机的ip和port
            std::string methodPath = servicePath + "/" + mp.first;
            char methodPathData[128] = {0};
            sprintf(methodPathData, "%s:%d", ip.c_str(), port);
            //通过ZOO_EPHEMERAL将/serviceName/methodName设置为临时节点
            zkClient_.create(methodPath.c_str(), methodPathData, strlen(methodPathData), ZOO_EPHEMERAL);
        }
    }

    //rpc服务端准备启动
    std::cout << "mrpcProvider start service at ip： " << ip << " port：" << port << std::endl;
    LOG_INFO("mrpcProvider start service at ip: %s, port: %d", ip.c_str(), port);
    //启动网络服务
    server.start();
    mrpcEventLoop.loop();
}

//新的socket连接回调
void mrpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        //和rpc client连接断开
        conn->shutdown();
    }
}

//已建立连接用户的读写事件回调
//如果远程有一个rpc调用请求，那么onMessage方法就会响应
//mrpcProvider和mrpcConsumer协商好通信用的protobuf数据类型 serviceName methodName args
//防止Tcp粘包问题，设置如下格式：
//size(4个字节)+serviceName+methodName+argSize[有size大小]+argStr[rpc方法参数]
void mrpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
                             muduo::net::Buffer *buffer,
                             muduo::Timestamp)
{
    //网络上接受远程rpc调用请求的字符流
    std::string recvBuf = buffer->retrieveAllAsString();
    //从字符流中读取
    uint32_t headerSize = 0;
    //读取二进制存储的整数
    recvBuf.copy((char *)&headerSize, 4, 0);
    //根据headerSize读取数据头的原始字符流。反序列化数据，得到mrpc请求的详细信息
    std::string mrpcHeaderStr = recvBuf.substr(4, headerSize);
    mrpc::rpcHeader rpcHeader_;
    std::string serviceName;
    std::string methodName;
    uint32_t argSize;
    if (rpcHeader_.ParseFromString(mrpcHeaderStr))
    {
        //数据头反序列化成功
        serviceName = rpcHeader_.servicename();
        methodName = rpcHeader_.methodname();
        argSize = rpcHeader_.argsize();
    }
    else
    {
        //数据头反序列化失败
        std::cout << "mrpcHeaderStr：" << mrpcHeaderStr << " parse error！" << std::endl;
        LOG_ERR("mrpcHeaderStr： %s parse error!", mrpcHeaderStr.c_str());
        return;
    }
    //获取rpc方法参数的字符流数据
    std::string argStr = recvBuf.substr(4 + headerSize, argSize);
    //打印调试信息
    std::cout << "***********************************" << std::endl;
    std::cout << "headerSize：" << headerSize << std::endl;
    std::cout << "serviceName：" << serviceName << std::endl;
    std::cout << "methodName：" << methodName << std::endl;
    std::cout << "argSize：" << argSize << std::endl;
    std::cout << "argStr：" << argStr << std::endl;
    std::cout << "***********************************" << std::endl;

    LOG_INFO("headerSize: %d", headerSize);
    LOG_INFO("serviceName: %s", serviceName.c_str());
    LOG_INFO("methodName: %s", methodName.c_str());
    LOG_INFO("argSize: %d", argSize);
    LOG_INFO("argStr: %s", argStr.c_str());

    //获取service对象和method对象
    auto serviceIt = mServiceMap.find(serviceName);
    if (serviceIt == mServiceMap.end())
    {
        std::cout << "serviceName：" << serviceName << "  not exist!" << std::endl;
        LOG_ERR("serviceName: %s not exist!", serviceName.c_str());
        return;
    }
    auto methodIt = serviceIt->second.mMethodMap.find(methodName);
    if (methodIt == serviceIt->second.mMethodMap.end())
    {
        std::cout << "methodName：" << methodName << "  not exist!" << std::endl;
        LOG_ERR("methodName: %s not exist!", methodName.c_str());
        return;
    }
    //service对象
    google::protobuf::Service *service = serviceIt->second.mService;
    //method对象
    const google::protobuf::MethodDescriptor *method = methodIt->second;
    //生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(argStr))
    {
        std::cout << "request parse error! content： " << argStr << std::endl;
        LOG_ERR("request parse error! content: %s", argStr.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    //给method方法的调用，绑定一个Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<mrpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &mrpcProvider::sendMrpcResponse,
                                                                                                 conn, response);
    //在框架上根据远端rpc请求，调用当前rpc节点发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

//Closure回调操作，用于序列化rpc的响应和网络发送
void mrpcProvider::sendMrpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string responseStr;
    //对response序列化
    if (response->SerializeToString(&responseStr))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送回mrpc调用方
        conn->send(responseStr);
    }
    else
    {
        std::cout << "serialize responseStr error!" << std::endl;
        LOG_ERR("serialize responseStr error!");
    }
    //模拟http的短连接服务将tcp设置为短连接，由mrpcProvider主动断开连接
    conn->shutdown();
}