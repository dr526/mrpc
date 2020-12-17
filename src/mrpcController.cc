#include "mrpcController.h"

mrpcController::mrpcController()
{
    failed = false;
    errText = "";
}

void mrpcController::Reset()
{
    failed = false;
    errText = "";
}

bool mrpcController::Failed() const
{
    return failed;
}

std::string mrpcController::ErrorText() const
{
    return errText;
}

void mrpcController::SetFailed(const std::string &reason)
{
    failed = true;
    errText = reason;
}

//目前尚未实现的具体功能
void mrpcController::StartCancel() {}
bool mrpcController::IsCanceled() const { return false; }
void mrpcController::NotifyOnCancel(google::protobuf::Closure *callback) {}