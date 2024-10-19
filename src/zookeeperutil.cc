#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器   zkserver给zkclient的通知
// 使用信号量同步连接操作，确保客户端在与服务器建立连接后再进行其他操作。
// void global_watcher(zhandle_t *zh, int type,
//                    int state, const char *path, void *watcherCtx)
// {
// 	// 获取上下文信息
// 	ZkClient *client = static_cast<ZkClient*>(watcherCtx);

//     if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型
// 	{
// 		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
// 		{
// 			sem_t *sem = (sem_t*)zoo_get_context(zh); // 获取信号量
//             sem_post(sem); // 发送信号，解除阻塞
// 		}
// 	}
// 	// 检测数据数据节点(方法名称)的变化
// 	else if(type == ZOO_CHANGED_EVENT)
// 	{
// 		std::cout << "Node data changed at: " << path << std::endl;
//         std::string data = client->GetData(path, true); // 重新获取数据并继续设置 Watcher
//         std::cout << "New data: " << data << std::endl;
// 	}
// 	// 子节点(具体的IP:PORT)发生变化
// 	else if(type == ZOO_CHILD_EVENT)
// 	{
// 		std::cout << "Children of node changed: " << path << std::endl;
//         client->WatchNodeChanges(path); // 重新监控子节点变化
// 	}
// }

void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
	// 获取上下文信息
	// ZkClient *client = static_cast<ZkClient*>(watcherCtx);

    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh); // 获取信号量
            sem_post(sem); // 发送信号，解除阻塞
		}
	}
}


ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start()
{
    // 获取ip + port
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	API调用线程 
	网络I/O线程  pthread_create  底层使用的是poll网络I/O
	watcher回调线程 pthread_create
    30000---->指的是超时时间
	*/
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    // 表示句柄的创建有没有成功-----》资源的开辟
	if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }
    /*
    *   信号量的初始值为 0，当全局 Watcher 观察到连接成功时，
    *   通过 sem_post 增加信号量，从而解除阻塞，表示连接已成功建立。
    */
    sem_t sem;
    sem_init(&sem, 0, 0);
	// 设置上下文
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem); // 等待被唤醒
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
    //  ZNODEEXISTS = -110, /*!< The node already exists */
    //  ZNONODE = -101, /*!< Node does not exist */
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        // 创建成功时，打印成功信息；否则，打印错误信息并退出程序。
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
       
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值
// 一个节点最大的数据是1M
std::string ZkClient::GetData(const char *path,bool watch)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
    // 同步获取与节点相关值的数据
	// int flag = zoo_get(m_zhandle, path, watch ? 1: 0, buffer, &bufferlen, nullptr);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	// 如果获取失败（返回值不为 ZOK），打印错误信息并返回空字符串。
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
     // 获取成功，返回获取的数据
	else
	{
		return buffer;
	}
}

// 获取指定路径下所有子节点的地址
std::vector<std::string> ZkClient::GetServiceAddresses(const char *path, bool watch)
{
    struct String_vector children;
    int flag = zoo_wget_children(m_zhandle, path, global_watcher, this, &children);

    std::vector<std::string> addresses;

    if (flag == ZOK) {
        std::cout << "Monitoring children of node: " << path << std::endl;
        for (int i = 0; i < children.count; ++i) {
            // std::string child_path = std::string(path) + "/" + children.data[i];
            // std::string child_data = GetData(child_path.c_str(), watch);
            // if (!child_data.empty()) {
            //     addresses.emplace_back(child_data);
            //     std::cout << "Service instance found: " << child_data << std::endl;
            // }
			// 直接将子节点的名字（即 ip:port）存储到 addresses 中
            std::string child_data = children.data[i];
            if (!child_data.empty()) {
                addresses.emplace_back(child_data);  // 添加 IP:Port
                std::cout << "Service instance found: " << child_data << std::endl;
            }
        }
    } else {
        std::cout << "Failed to set watch on children of node: " << path 
                  << ", error code: " << flag << std::endl;
    }
    return addresses;
}




// 监控子节点的变化
void ZkClient::WatchNodeChanges(const char *path)
{
    struct String_vector children;

	int flag = zoo_wget_children(m_zhandle, path, global_watcher, this, &children);

	if (flag != ZOK)
	{
		std::cout << "Monitoring children of node: " << path << std::endl;
        for (int i = 0; i < children.count; ++i) {
            std::cout << "Child node: " << children.data[i] << std::endl;
        }
	}
	else{
		std::cout << "Failed to set watch on children of node: " << path << std::endl;
	}
}