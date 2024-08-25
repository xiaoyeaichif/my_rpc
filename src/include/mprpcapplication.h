#pragma once

class MprpcApplication
{
public:
    static void Init(int argc,char ** argv);
    static MprpcApplication &GetInstance(); // 获取唯一的单例
private:
    MprpcApplication(){}
    // 禁止拷贝构造函数
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication&&) = delete;// 禁止移动构造函数
    // 禁止赋值函数
    MprpcApplication &operator = (const MprpcApplication &) = delete;
};