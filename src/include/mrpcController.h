#ifndef MRPCCONTROLLER_H
#define MRPCCONTROLLER_H

#include <google/protobuf/service.h>
#include <string>

class mrpcController : public google::protobuf::RpcController
{
public:
    mrpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string &reason);

    //目前尚未实现的具体功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure *callback);

private:
    bool failed;         //rpc方法执行过程中的状态
    std::string errText; //rpc方法执行过程中的错误信息
};

#endif //MRPCCONTROLLER_H