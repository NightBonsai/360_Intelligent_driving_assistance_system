#pragma once

#include <string.h>
#include "IPC.h"

#define PACKET_SIZE 2048

//业务基类：要有两个子类（前置->后置；后置->前置）
class BaseTask
{
public:
	BaseTask(char* data);
	virtual~BaseTask();			//虚析构函数——执行子类析构时会执行父类析构

	virtual void work() = 0;	//纯虚函数——具体业务子类实现

	//公共接口
	void setWorkfd(int val);
	void setIPC(IPC* ipc);
protected:
	char data[PACKET_SIZE];		//业务包数据
	int workfd;

	IPC* ipc;					//共享内存通信类
};
