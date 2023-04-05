#pragma once

#include <netinet/in.h>	//sockaddr_in包
#include <arpa/inet.h>	//sockaddr_in包

//网络地址类：存储连接的服务器信息
class HostAddress
{
public:
	HostAddress(unsigned short port);
	~HostAddress();

	//公有接口
	struct sockaddr_in getAddr_in();
	struct sockaddr* getAddr();
	unsigned short getPort();
	void setPort(unsigned short port);
	int getLength();

private:
	struct sockaddr_in addr;	//IP地址、协议族
	unsigned short port;		//端口号
	int length;
};

