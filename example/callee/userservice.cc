
#include <iostream>
#include <string>
#include "user.pb.h"



/*
    UserService原来是一个本地服务，提供两个进程内的本地方法，Login和GetFriendLists
*/

class UserService : public fixbug::UserServiceRpc // 使用在Rpc服务发不端
{
public:
    bool Login(std::string name , std::string pwd)
    {
        std::cout<<"doing local service: Login"<<std::endl;
        std::cout<<"name"<< name <<"pwd"<< pwd <<std::endl;
        return true;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller   ===>   Login(LoginRequest)  => muduo =>   callee 
    2. callee   ===>    Login(LoginRequest)  => 交到下面重写的这个Login方法上了
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name(); // 获取的是远端的方法
        std::string pwd  = request->pwd();
        
        // 做本地业务
        bool login_result = Login(name, pwd); 

        // 把响应写入  包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errorcode(0); // 0代表没有错误
        code->set_errormsg("");
        response->set_success(login_result);

        // 执行回调操作   执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run(); // 需要重写Closure的纯虚函数
    }
};

int main(int argc,char **argv)
{
    // 调用框架的初始化操作
    // 静态函数
    // MprpcApplication::Init(argc, argv);

    // // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    // RpcProvider provider;
    // provider.NotifyService(new UserService());

    // // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    // provider.Run();

    return 0;
}