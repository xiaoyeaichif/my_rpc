#pragma once
#include "mprpcconfig.h"

// 单例模式
// mprpc的框架类,负责初始化任务
class MprpcApplication
{
public:
    static void Init(int argc,char ** argv);
    static MprpcApplication &GetInstance(); // 获取唯一的单例
private:
    static MprpcConfig m_config; 
    MprpcApplication(){}
    // 禁止拷贝构造函数
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication&&) = delete;// 禁止移动构造函数
    // 禁止赋值函数
    MprpcApplication &operator = (const MprpcApplication &) = delete;
};