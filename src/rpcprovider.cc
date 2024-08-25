#include "rpcprovider.h"
#include <string>
#include "mprpcapplication.h"


/*
简单来说，服务提供方需要知道：
    1：自己提供了哪些服务（服务名称）。
    2：每个服务包含哪些方法（方法名称和数量）。
    3：每个方法的详细描述（包括方法的参数、返回值等信息）
*/

/*
service_name =>  service描述   
                        =》 service* 记录服务对象
                        method_name  =>  method方法对象

常见的序列化方式：json   protobuf
*/

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    // 获取服务的描述
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    // 提供了服务的元数据信息，比如服务名称、方法数量等。
    // auto 变量为 google::protobuf::ServiceDescriptor
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名称
    std::string service_name = pserviceDesc->name();
    // 获取该服务的数量
    int methodCnt = pserviceDesc->method_count();

    // 获取服务名
    std::cout << "service_name:" << service_name << std::endl;
    
    
    for (int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象指定下标的服务方法描述（抽象描述）
        // 获取索引为 i 的方法的描述符（MethodDescriptor）。
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        // 获取方法的名称
        std::string method_name = pmethodDesc->name();
        // 将方法名称与方法描述符映射在一起
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        // 获取方法名
        std::cout << "method_name:" << method_name << std::endl;
    }
    // 将服务名称与ServiceInfo映射在一起
    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});
    
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
    // 读写回调
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