#include "rpcprovider.h"
#include <string>
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <iostream>

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
// 短连接请求
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr&conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args    定义proto的message类型，进行数据头的序列化和反序列化
                                 service_name method_name args_size
16UserServiceLoginzhang san123456   

header_size(4个字节) + header_str + args_str
10 "10"
10000 "1000000"
std::string   insert和copy方法 ----》按内存读取
*/




// 已建立连接用户的读写事件回调
// 一旦有读写事件，立刻给读写事件回调处理
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&conn, 
                            muduo::net::Buffer*buffer, muduo::Timestamp)
{
    // 接收网络中的数据
    std::string recv_buf = buffer->retrieveAllAsString();

    // 解析网络中的数据
    // 防止粘包的问题,区分消息头长度 + 消息体 + 内容

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

   
    
    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    // rpc_header_str原始字符串
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    // 服务名称
    std::string service_name;
    // 方法名称
    std::string method_name;
    // args_size: 指示方法参数的字符流数据长度，
    uint32_t args_size;

    // 数据反序列化
    // 网络中的数据为二进制数据------》然后反序列化为对象
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else{
        // 反序列化失败
        std::cout<<"rpc_header_str:" << rpc_header_str << " parse error!"<<std::endl;
        return; // 反序列化失败，直接结束
    }

    // 获取rpc方法参数的字符流数据---》此时还是二进制数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    // 检测该客户端请求的服务是不是在服务端
    auto it = m_serviceMap.find(service_name);
    // 表示服务端并没有这个服务名
    // 结束
    if(it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    // 服务存在，但是服务下面的方法并不存在
    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    // 请求服务存在，并且请求的方法名也存在
    google::protobuf::Service *service = it->second.m_service; // 获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象  Login
    
    // 生成rpc方法调用的请求request和响应response参数
    // 请求
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    // 将得到的args_str反序列化-----》将二进制数据-----》转变为对象
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }

    // 响应----》这个数据是要回复客户端的请求的
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 回调函数的使用---->将respone对象序列化传递到网络中
    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // response进行序列化
    {
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl; 
    }
    conn->shutdown(); // 模拟http的短链接服务，由rpcprovider主动断开连接
}