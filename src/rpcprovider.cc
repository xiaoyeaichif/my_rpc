#include "rpcprovider.h"
#include <string>
#include "mprpcapplication.h"

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{

}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    // 读取配置文件
    // 获取ip地址
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // 获取端口号
    // 将字符转为整数
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // muduo网络库的使用
    muduo::net::InetAddress address(ip,port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");

    // 设置连接与信息处理回调方法
    // 用muduo库的好处是分离了网络代码和业务代码
    server.setConnectionCallback(bind(&RpcProvider::OnConnection,this,std::placeholders:: _1));
    server.setMessageCallback(bind(&RpcProvider::OnMessage,this,
    std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    // 设置线程的数量
    // 一个连接线程，3个处理线程
    server.setThreadNum(4);

    // rpc服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop(); 
}


// 新的socket连接回调
// 一旦有连接进来，立刻给连接回调处理
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr&)
{

}
// 已建立连接用户的读写事件回调
// 一旦有读写事件，立刻给读写事件回调处理
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp)
{

}