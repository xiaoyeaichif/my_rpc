#pragma once
#include <google/protobuf/service.h>
#include <string>

/*
*   google::protobuf::RpcController----> 用于控制RPC调用的行为和状态
*   本类用于在 RPC 方法调用过程中跟踪调用的状态（如成功或失败）和错误信息。
*/


class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController(); // 构造函数，用于初始化m_failed和m_errText
    // 重置控制器的状态。将 m_failed 设置为 false 并清空 m_errText。
    // 这使得控制器可以被重复使用。
    void Reset(); 
    // 用于检查 RPC 调用是否失败。
    bool Failed() const;
    // 用于获取 RPC 调用失败时的错误信息。
    std::string ErrorText() const;
    // 设置 m_failed 为 true，并设置 m_errText 为指定的错误信息 reason，
    // 表示 RPC 调用失败并提供错误原因。
    void SetFailed(const std::string& reason);

    // 目前未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed; // RPC方法执行过程中的状态  true 和 false
    std::string m_errText; // RPC方法执行过程中的错误信息
};