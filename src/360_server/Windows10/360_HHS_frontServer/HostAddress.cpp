#include "HostAddress.h"

HostAddress::HostAddress(unsigned short port)
{
	//网络初始化
	this->port = port;

	this->addr.sin_family = AF_INET;			//网络连接协议族 IPv4
	this->addr.sin_addr.s_addr = INADDR_ANY;	//服务器IP地址不会轻易变动，开放服务器IP和端提供给客户端主动来连接
												//系统自动获取本地联网IP
	this->addr.sin_port = htons(this->port);	//开放服务器端口，提供给客户端来主动连接，因为内存保存数据有大小端区分，所以用htons转换	

	this->length = sizeof(this->addr);
}

HostAddress::~HostAddress()
{
}

sockaddr_in HostAddress::getAddr_in()
{
	return this->addr;
}

sockaddr* HostAddress::getAddr()
{
	return (struct sockaddr*)&(this->addr);
}

unsigned short HostAddress::getPort()
{
	return this->port;
}

void HostAddress::setPort(unsigned short port)
{
	this->port = port;
}

int HostAddress::getLength()
{
	return this->length;
}
