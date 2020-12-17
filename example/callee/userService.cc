#include <iostream>
#include <string>
#include "user.pb.h"
#include "mrpcApplication.h"
#include "mrpcProvider.h"
#include "mrpcLogger.h"

//userService是一个本地服务，提供了两个进程的本地方法
class userService : public dr526::userServiceRpc
{
public:
    bool login(std::string name, std::string pwd)
    {
        std::cout << "execute local service: login" << std::endl;
        std::cout << "name: " << name << " pwd： " << pwd << std::endl;
        if (name == "dr526" && pwd == "123456")
        {
            return true;
        }
        return false;
    }

    bool registe(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "execute local service registe" << std::endl;
        std::cout << "id: " << id << "name: " << name << "pwd: " << pwd << std::endl;
        return true;
    }

    //重写基类userServiceRpc的虚函数
    //下面这个方法是给框架直接调用的
    //protobuf自动进行了反序列化，并存放在request中
    void login(::google::protobuf::RpcController *controller,
               const ::dr526::loginRequest *request,
               ::dr526::loginResponse *response,
               ::google::protobuf::Closure *done)
    {
        //1. 框架给业务上报了请求参数loginRequest,业务获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //2. 调用本地方法，执行本地业务
        bool loginResult = login(name, pwd);

        //3. 组装响应信息，包括错误码、错误消息、返回值
        dr526::resultCode *result = response->mutable_result();
        if (loginResult)
        {
            result->set_errcode(1);
            result->set_errmsg("登录成功");
        }
        else
        {
            result->set_errcode(0);
            result->set_errmsg("登录失败");
        }
        response->set_success(loginResult);

        //4. 执行回调操作，包括响应对象的序列化和网络发送
        done->Run();
    }

    void registe(::google::protobuf::RpcController *controller,
                 const ::dr526::registeRequest *request,
                 ::dr526::registeResponse *response,
                 ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool registeResult = registe(id, name, pwd);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("注册成功");
        response->set_success(registeResult);
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message");
    LOG_INFO("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    //调用框架的初始化操作
    mrpcApplication::init(argc, argv);

    //发布服务，即将userService对象发布到rpc节点上
    //provider是一个rpc网络服务对象
    mrpcProvider provider;
    provider.notifyService(new userService());

    //启动一个rpc服务发布节点。
    //Run()以后，进程进入阻塞状态，的呢古代远程的rpc调用请求
    provider.run();
}