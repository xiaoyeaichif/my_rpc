#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

// 类外初始化静态成员变量
MprpcConfig MprpcApplication::m_config;

// 输出错误的日志
void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>"<<std::endl;
}


void MprpcApplication::Init(int argc,char ** argv)
{
    if(argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    /*
        while循环：getopt函数每次被调用时，都会返回下一个选项的字符，
        直到所有选项都解析完毕。当所有选项解析完毕后，getopt返回-1，退出循环。
    */
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        //不希望出现的参数
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        //没有带正确的参数
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件了 rpcserver_ip=  rpcserver_port   zookeeper_ip=  zookepper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // 打印检测
    std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
}

MprpcApplication & MprpcApplication::GetInstance() // 获取唯一的单例
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}