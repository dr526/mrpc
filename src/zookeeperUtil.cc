#include <iostream>
#include "zookeeperUtil.h"
#include "mrpcApplication.h"
#include "mrpcLogger.h"
//全局的watcher观察器
//zookeeper server给zookeeper client的通知
void globalWatcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    //回调的信息类型是和会话相关的消息类型
    if (type == ZOO_SESSION_EVENT)
    {
        //zookeeper client和zookeeper server连接成功
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

zkClient::zkClient() : zhandle(nullptr) {}

zkClient::~zkClient()
{
    if (zhandle != nullptr)
    {
        //关闭句柄，释放资源
        zookeeper_close(zhandle);
    }
}

//连接zookeeper server
void zkClient::start()
{
    //获取配置文件中的zookeeper server ip地址和端口号
    std::string host = mrpcApplication::getInstance().getMrpcConfig().load("zookeeperIp");
    std::string port = mrpcApplication::getInstance().getMrpcConfig().load("zookeeperPort");
    std::string connectStr = host + ":" + port;
    //获取句柄
    //zookeeper_mt多线程版本的zookeeper的API客户端程序提供三个线程
    //API调用线程
    //网络I/O线程
    //watcher回调线程
    //zookeeper连接的建立是异步的
    //超时时间为30000ms，即：30s
    zhandle = zookeeper_init(connectStr.c_str(), globalWatcher, 30000, nullptr, nullptr, 0);
    if (zhandle == nullptr)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        LOG_ERR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }
    //定义信号量并初始化
    sem_t sem;
    sem_init(&sem, 0, 0);
    //给句柄设置信号量
    zoo_set_context(zhandle, &sem);
    sem_wait(&sem);
    std::cout << "zookeeper_init success" << std::endl;
    LOG_INFO("zookeeper_init success");
}
//添加节点和相应数据，相当于命令create
//state是区分临时节点还是永久节点
//state=0默认是永久节点
void zkClient::create(const char *path, const char *data, int dataLen, int state)
{
    char pathBuffer[128];
    int bufferLen = sizeof(pathBuffer);
    int flag;
    //判断相应路径的节点是否存在，如果存在就不再重复创建
    flag = zoo_exists(zhandle, path, 0, nullptr);
    //表明相应路径的znode节点不存在
    if (flag == ZNONODE)
    {
        //创建指定path的znode节点
        flag = zoo_create(zhandle, path, data, dataLen,
                          &ZOO_OPEN_ACL_UNSAFE, state, pathBuffer, bufferLen);
        if (flag == ZOK)
        {
            std::cout << "znode create success! path: " << path << std::endl;
            LOG_INFO("znode create success! path: %s", path);
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error! path: " << path << std::endl;
            LOG_ERR("flag: %d, znode create error! path: %s", flag, path);
            exit(EXIT_FAILURE);
        }
    }
}

//获取节点数据，相当于命令get
//根据指定的path,获取znode节点的值
std::string zkClient::getData(const char *path)
{
    char buffer[64];
    int bufferLen = sizeof(buffer);
    int flag = zoo_get(zhandle, path, 0, buffer, &bufferLen, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get znode error! path: " << path << std::endl;
        LOG_ERR("get znode error! path: %s", path);
        return "";
    }
    else
    {
        return buffer;
    }
}