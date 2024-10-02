
// 服务提供
#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>

// 服务的注册
class FriendService : public fixbug::FriendServiceRpc
{
public:
    // 本地服务
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service! userid:" << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }
     // 重写基类方法
    void GetfriendLists(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetfriendListsRequest* request,
                       ::fixbug::GetfriendListsResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 获取参数
        uint32_t userid = request->userid();
        std::vector<std::string> FriendList = GetFriendsList(userid);
        // 写入响应消息
        // 设置错误信息以及错误代码
        response->mutable_result()->set_errorcode(0);
        response->mutable_result()->set_errormsg("");
        // 查看有多少个friend
        for (std::string &name : FriendList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
        // 执行回调操作
        // 执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

// 主函数的实现
int main(int argc,char ** argv)
{
    if (argc < 2)
    {
        std::cout << "run: " << argv[0] << " ip_port" << std::endl;
        return -1;
    }
    // 服务的注册
    // 调用框架的初始化操作
    // 静态函数
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    // 是一个同步过程
    provider.Run();

    return 0;
}