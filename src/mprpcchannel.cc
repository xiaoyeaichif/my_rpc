#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"


// 前置工具函数:生成缓存的键
std::string getCacheKey(const std::string& service_name, const std::string& method_name)
{
    return service_name + "::" + method_name;
}


/*
header_size + service_name method_name args_size + args
*/
// 所有通过stub代理对象调用的rpc方法，都走到这里了，
// 统一做rpc方法调用的数据数据序列化和网络发送 
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf:: Closure* done)
{
    // 这些信息用于构建 RPC 请求的头部，明确告诉服务器要调用哪个服务的哪个方法。
    const google::protobuf::ServiceDescriptor* sd = method->service();
    // 明确调用哪个服务
    std::string service_name = sd->name(); // service_name
    // 明确调用哪个方法
    std::string method_name = method->name(); // method_name

    // 检查本地是否存在缓存，如果存在，则直接调用
    std::string cacheKey = getCacheKey(service_name, method_name);
    // 用来存放ip:port
    std::string host_ip_port; 

    // 检查缓存是否存在
    // 防止线程间的竞争问题
    // 智能锁出作用域会自动析构
    {
        std::lock_guard<std::mutex> lock(m_Cache_mutex);
        // 检查缓存是否存在
        auto it = m_serviceCache.find(cacheKey);
        // 缓存存在，直接使用缓存建立连接
        if(it != m_serviceCache.end())
        {
            host_ip_port = it->second;
            // 输出缓存的信息检查
            std::cout<<"Cache hit for key: "<< cacheKey <<", host_ip_port: "
                    << host_ip_port <<std::endl;
        }
    }
    // 缓存未命中,要从Zookeeper上获取服务器地址
    if(host_ip_port.empty())
    {
        std::cout << "Cache miss for key: " << cacheKey << ", querying ZooKeeper..." << std::endl;
        // 从zookeeper服务器上获取服务器地址
        ZkClient zkCli;
        zkCli.Start();
        // 获取服务名称和服务方法
        std::string path = "/" + service_name + "/" + method_name;
        // 检查改服务方法下面是否有数据存在
        std::string host_data = zkCli.GetData(path.c_str());

         // 检测数据是否为空，如果为空直接返回
        if(host_data.empty())
        {
            controller->SetFailed(path + " is not exist!");
            std::cerr << "Error: " << path << " does not exist in ZooKeeper!" << std::endl;
            return;
        }

        // 数据不为空, 检查ip:port格式是否正确
        size_t index = host_data.find(":");
        if(index == std::string::npos)
        {
            controller->SetFailed(path + " format is invalid!");
            std::cerr << "Error: " << path << " format is invalid!" << std::endl;
            return;
        }

        // ip:port正确的格式
        // 将ip:port正确的格式存进本地缓冲中
        std::string ip = host_data.substr(0, index);
        // 获取端口
        uint16_t port = atoi(host_data.substr(index + 1).c_str());

        // 更新缓存信息
        {
            std::lock_guard<std::mutex> lock(m_Cache_mutex);
            // 更新缓存信息
            m_serviceCache[cacheKey] = host_data;
            // 输出缓存更新成功信息
            std::cout << "Updated cache for key: " << cacheKey << " with host: " << host_data << std::endl;
            // 将 host_ip_port 设置为 host_data 以供后续使用
            // host_ip_port = host_data; 
        }
    }

    // 在这里其实可以加上负载均衡的策略,这样就可以选择合适的节点进行连接

    // 获取当前节点的ip和port
    // 从哈希表中直接获取
    host_ip_port = m_serviceCache[cacheKey];
    int index_new = host_ip_port.find(":");
    // 检查是否满足ip:port的格式
    if(index_new == -1 )
    {
        controller->SetFailed(method_name + " address is invalid!");
        return;
    }
    // 表示当前合格
    std::string ip = host_ip_port.substr(0, index_new);
    std::uint16_t port = atoi(host_ip_port.substr(index_new + 1).c_str());

    // 获取参数的序列化字符串长度 args_size----》序列化请求参数
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        // args_size 保存了序列化后的请求参数的长度
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        // std::cout<<"serialize request error!"<<std::endl;
        return;
    }
    
    // 定义rpc的请求header-----》构建并序列化 RPC 请求头部
    // rpcHeader 包含三部分---》服务名（service_name）、方法名（method_name）
    // 和请求参数大小（args_size）
    // 这部分数据是要通过网络传输的
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // rpcHeader 对象被序列化为字符串 rpc_header_str，并计算其长度 header_size。
    uint32_t header_size = 0;
    std::string rpc_header_str;
    //序列化成功
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpc header error!");
        // std::cout<<"serialize rpc header error!"<<std::endl;
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    //从0开始写四个字节
    // 这段4字节的长度是为了粘包而设置的
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str; // rpcheader RPC 请求头部的序列化字符串。
    send_rpc_str += args_str; // args 代表RPC请求参数数据

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str.size() << std::endl; 
    std::cout << "============================================" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    // 建立连接失败
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        std::cout <<"create socket error! "<< errno <<std::endl;
        exit(EXIT_FAILURE);
    }

    // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    // 如果改为注册中心获取的话，需要改变代码的逻辑
    // ZkClient zkCli;
    // zkCli.Start();

    // // 获取服务名称和服务方法
    // std::string path = "/" + service_name + "/" + method_name;
    // // 检查改服务方法下面是否有数据存在
    // std::string host_data = zkCli.GetData(path.c_str());

    // // 检测数据是否为空，如果为空直接返回
    // if(host_data.empty())
    // {
    //     controller->SetFailed(path + " is not exist!");
    //     return;
    // }

    // // 数据存在，将字符串按照:分割
    // int index = host_data.find(":");
    // // 检查ip格式正不正确
    // if(index == -1 )
    // {
    //     controller->SetFailed(method_name + " address is invalid!");
    // }
    // // ip:port格式正确，获取ip和port
    // std::string ip = host_data.substr(0, index).c_str();
    // uint16_t port = atoi(host_data.substr(index+1).c_str());
    

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        std::cout <<"connect error! errno: "<<errno<<std::endl;
        return;
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        std::cout <<"send error! errno: "<<errno<<std::endl;
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        close(clientfd);
        std::cout <<"recv error! errno: "<<errno<<std::endl;
        return;
    }

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size); // bug出现问题，recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        std::cout <<"parse error! "<<recv_buf<<std::endl;
        close(clientfd);
        return;
    }

    // 关闭套接字
    close(clientfd);
}