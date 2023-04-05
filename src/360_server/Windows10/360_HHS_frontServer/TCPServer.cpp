#include "TCPServer.h"

TCPServer::TCPServer(unsigned short port) :BaseSocket(port)
{
	//初始化
	this->address = new HostAddress(port);
}

TCPServer::~TCPServer()
{
	delete this->address;
}

void TCPServer::stopConnect()
{
	if (this->socketfd != 0 && this->socketfd > 0)
	{
		close(this->socketfd);
		this->socketfd = 0;
	}
}

void TCPServer::work()
{
	//端口复用—避免出现address already be used
	int optval = 1;
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval));

	//绑定端口和IP地址
	if (-1 == bind(socketfd, this->address->getAddr(), this->address->getLength()))
	{
		perror("bind error");
	}
	//监听是否有用户上线
	if (-1 == listen(socketfd, SOCKET_MAX_NUM))
	{
		perror("listen error");
	}
	cout << "网络搭建成功" << endl;
}

HostAddress* TCPServer::getAddress()
{
	return this->address;
}

void TCPServer::setAddress(HostAddress* address)
{
	this->address = address;
}
