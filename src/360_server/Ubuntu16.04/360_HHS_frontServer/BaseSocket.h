#pragma once

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>	//IPPROTO_TCP头文件
#include <stdio.h>

//网络基类
class BaseSocket
{
public:
	BaseSocket(unsigned short port);
	~BaseSocket();

	void startConnect();		//网络初始化

	//公有接口
	int getSocketfd();

	//纯虚函数—交由子类实现
	virtual void stopConnect() = 0;	//断开连接
	virtual void work() = 0;		//子类实现各功能			
protected:
	int socketfd;	//socket文件描述符
};

