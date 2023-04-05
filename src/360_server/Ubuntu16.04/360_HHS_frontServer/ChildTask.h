#pragma once

#include <iostream>
#include <time.h>
#include <unistd.h>
#include "BaseTask.h"
#include "protocol.h"
#include "CRC.h"

#define PACKET_SIZE 2048    //业务包大小&共享内存一个数据区大小
#define SEND_TO_BACK 1      //业务类型：接收客户端业务包
#define GET_FROM_BACK 2     //业务类型：接收后置服务器返回包

using namespace std;

class ChildTask :
    public BaseTask
{
public:
    ChildTask(char* data, int type);
    ~ChildTask();

    //纯虚函数重定义：执行前置服务器业务
    void work();

    //业务处理函数
    void toBackServer();        //业务包丢给共享内存，后置服务器接收
    void toClient();            //从后置服务器接收返回包，返回包丢给客户端

    void currentLogOutput(const REBUILD_HEAD head, const char newPacket[]);    //实时日志输出
private:
    int index;              //记录当前要读取数据区对应的索引区下标
    int businessType;       //记录当前要执行的业务类型

    REBUILD_HEAD rsvHead;   //记录从共享内存数据区读到的业务返回包头
};

