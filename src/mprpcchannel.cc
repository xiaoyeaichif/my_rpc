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
// #include "mprpccontroller.h"
// #include "zookeeperutil.h"

/*
header_size + service_name method_name args_size + args
*/
// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送 
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
        // controller->SetFailed("serialize request error!");
        std::cout<<"serialize request error!"<<std::endl;
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
        //controller->SetFailed("serialize rpc header error!");
        std::cout<<"serialize rpc header error!"<<std::endl;
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
    std::cout << "args_str: " << args_str << std::endl; 
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
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    

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