

// 服务调用方
#include "friend.pb.h"
#include <iostream>
#include "mprpcapplication.h"


// 主函数的调用部分
int main(int argc, char** argv)
{
    // 初始化框架
    MprpcApplication::Init(argc, argv);

    // 使用stub代理对象就行远程rpc的调用
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // 进行数据的序列化操作
    fixbug::GetfriendListsRequest request;
    request.set_userid(10000);

    // 响应的反序列化
    // 响应的信息都在response中
    fixbug::GetfriendListsResponse response;

    // 定义一个控制对象,获取在调用时候的状态信息
    MprpcController controller;
    // 进行调用
    stub.GetfriendLists(&controller, &request, &response, nullptr);

    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else{
        // 检查调用是否失败
        if (0 == response.result().errorcode())
        {
            //调用正确
            std::cout << "rpc GetfriendLists response success!" << std::endl;
            // 输出好友的列表个数
            int size = response.friends_size();
            for(int i = 0;i < size;i++)
            {
                std::cout << "index:" << (i+1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            //调用失败
            std::cout << "rpc GetfriendLists response error : " << response.result().errormsg() << std::endl;
        }
    }
    return 0;
}