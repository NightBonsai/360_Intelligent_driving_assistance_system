#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <string>
#include <time.h>
#include <list>
#include <iterator>
#include <algorithm>
#include "BaseTask.h"
#include "protocol.h"
#include "DBBusiness.h"

#define PACKET_SIZE 2048    //业务包大小&共享内存数据区大小

using namespace std;

class ChildTask :
    public BaseTask
{
public:
    ChildTask(char* data);
    ~ChildTask();

    //纯虚函数重定义：从共享内存提取数据
    void work();

    //业务处理函数
    void loginBusiness();       //登录业务
    void registBusiness();      //注册业务
    void photoUploadBusiness(); //图片数据上传业务
    void photoSearchBusiness(); //图片信息检索业务
    void videoUploadBusiness(); //视频数据上传业务
    void videoSearchBusiness(); //视频信息检索业务

    void addLogBusiness(const char operationRecord[],const char operationPath[]);   //全部日志写入业务  

    void getTime(char buffer[]);                    //获取当前时间
    bool createDirs(const std::string& dirName);    //创建新保存图片多级目录

    //公共接口
    void setIndex(int val);
private:
    int index;                  //记录当前要读取数据区对应的索引区下标
    char operationPath[80];     //记录当前操作的本地路径

    REBUILD_HEAD rsvHead;       //记录从共享内存数据区读到的业务包头
};

