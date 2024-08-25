#include "mprpcconfig.h"

#include <iostream>
#include <string>
 // 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 打开配置文件
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 文件打开成功
    // 1.注释--->首字母是不是#号   2.正确的配置项 =    3.去掉开头的多余的空格 
    while(!feof(pf))
    {
        char buf[512] = {0};
        // 读取一行
        fgets(buf, 512, pf);

        // 去掉字符串前面,后面多余的空格
        std::string read_buf(buf);
        Trim(read_buf);
        
        // 判断#的注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            // 配置项不合法
            continue;
        }

        // 合法的键值对
        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        // 去除key前后的空格
        Trim(key);
        //去掉字符后面的换行符
        // 比如"127.0.0.1\n"  "127.0.0.1    \n"
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx+1, endidx-idx-1);
        // 去掉value前后的空格
        Trim(value);
        m_configMap.insert({key,value});

    }


}
// 查询配置项信息
// 不要使用[]----->这个如果在哈希表中不存在，会插入该元素
std::string MprpcConfig::Load(const std::string &key)
{
     auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}




// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    // 查找第一个不是空格的位置
    // -1代表没有找到，也就是说全是空格
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格
        // 比如---abc---,-代表一个空格
        // src_buf经过处理后现在变成abc---
        src_buf = src_buf.substr(idx, src_buf.size()-idx);
    }
    // 去掉字符串后面多余的空格
    // 查找后面第一个不是空格的字符
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        // abc---变成abc
        src_buf = src_buf.substr(0, idx+1);
    } 
}