#include "BaseSocket.h"

BaseSocket::BaseSocket(unsigned short port)
{
	//初始化
	this->socketfd = 0;
}

BaseSocket::~BaseSocket()
{
}

void BaseSocket::startConnect()
{
	//网络初始化：判断是否可以搭建网络
	//						  协议族	TCP协议		默认
	this->socketfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (0 > this->socketfd)
	{
		perror("socket error");
	}
	else
	{
		this->work();
	}
}

int BaseSocket::getSocketfd()
{
	return this->socketfd;
}
