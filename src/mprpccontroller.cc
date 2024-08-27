#include "mprpccontroller.h"


MprpcController::MprpcController() // 构造函数，用于初始化m_failed和m_errText
{
    m_failed = false;
    m_errText = "";
}
// 重置控制器的状态。将 m_failed 设置为 false 并清空 m_errText。
// 这使得控制器可以被重复使用。
void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}
// 用于检查 RPC 调用是否失败。
bool MprpcController::Failed() const
{
    return m_failed;
}
// 用于获取 RPC 调用失败时的错误信息。
std::string MprpcController::ErrorText() const
{
    return m_errText;
}
// 设置 m_failed 为 true，并设置 m_errText 为指定的错误信息 reason，
// 表示 RPC 调用失败并提供错误原因。
void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true; //表示有错误
    m_errText = reason;
}


// 目前未实现具体的功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}