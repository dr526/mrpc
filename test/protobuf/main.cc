#include "test.pb.h"
#include <iostream>
#include <string>
using namespace dr526;

int main()
{
    //protobuf简单的message
    //封装了loginRequest对象的数据
    loginRequest req;
    req.set_name("liguojun");
    req.set_pwd("123456");

    //对象数据序列化
    std::string reqStr;
    if (req.SerializeToString(&reqStr))
    {
        std::cout << reqStr << std::endl;
        std::cout << "*******************" << std::endl;
    }

    //将reqStr进行反序列化为loginRequest请求对象
    loginRequest req_;
    if (req_.ParseFromString(reqStr))
    {
        std::cout << req_.name() << std::endl;
        std::cout << req_.pwd() << std::endl;
    }

    //protobuf的message中嵌套message
    //封装loginResponse对象的数据
    loginResponse rsp;
    resultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("登录失败");
    rsp.set_success(false);

    //对象数据的序列化
    std::string rspStr;
    if (rsp.SerializeToString(&rspStr))
    {
        std::cout << rspStr << std::endl;
        std::cout << "*******************" << std::endl;
    }

    //将rspStr进行反序列化为loginResponse响应对象
    loginResponse rsp_;
    if (rsp_.ParseFromString(rspStr))
    {
        std::cout << rsp_.result().errmsg() << std::endl;
        std::cout << rsp_.result().errcode() << std::endl;
        std::cout << rsp_.success() << std::endl;
    }

    //protobuf中message包含一个列表
    //封装getFriendListsResponse对象的数据
    getFriendListsResponse getFLists;
    resultCode *rc_ = getFLists.mutable_result();
    rc_->set_errcode(1);
    rc_->set_errmsg("获取朋友列表成功");
    user *user1 = getFLists.add_friendlist();
    user1->set_name("liguojun");
    user1->set_age(23);
    user1->set_sx(user::male);
    user *user2 = getFLists.add_friendlist();
    user2->set_name("dr526");
    user2->set_age(20);
    user2->set_sx(user::female);

    //对象数据的序列化
    std::string getFListsStr;
    if (getFLists.SerializeToString(&getFListsStr))
    {
        std::cout << getFListsStr << std::endl;
        std::cout << "*******************" << std::endl;
    }

    //将getFListsStr进行反序列化为getFriendListsResponse响应对象
    getFriendListsResponse getFLists_;
    if (getFLists_.ParseFromString(getFListsStr))
    {
        std::cout << getFLists_.result().errmsg() << std::endl;
        std::cout << getFLists_.result().errcode() << std::endl;
        int size = getFLists_.friendlist_size();
        for (int i = 0; i < size; ++i)
        {
            user usr = getFLists_.friendlist(i);
            std::cout << usr.name() << std::endl;
            std::cout << usr.age() << std::endl;
            std::cout << usr.sx() << std::endl;
        }
    }
    return 0;
}