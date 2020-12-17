#ifndef ZOOKEEPERUTIL_H
#define ZOOKEEPERUTIL_H

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

//封装zookeeper客户端
class zkClient
{
public:
    zkClient();
    ~zkClient();
    //连接zookeeper server
    void start();
    //添加节点和相应数据，相当于命令create
    void create(const char *path, const char *data, int dataLen, int state = 0);
    //获取节点数据，相当于命令get
    std::string getData(const char *path);

private:
    zhandle_t *zhandle; //zk客户端句柄
};

#endif //ZOOKEEPERUTIL_H