#include "mprpcapplication.h"

void MprpcApplication::Init(int argc,char ** argv)
{

}

MprpcApplication & MprpcApplication::GetInstance() // 获取唯一的单例
{
    static MprpcApplication app;
    return app;
}