#pragma once

#include <sys/epoll.h>
#include <string.h>
#include <time.h>
#include <map>
#include "ThreadPool.h"		
#include "TCPServer.h"
#include "ChildTask.h"
#include "IPC.h"

#define EPOLL_SIZE 10
#define PACKET_SIZE 2048
#define HEART_BEAT_TIME 300		//心跳业务测试时间段：5分钟

//心跳检测结构体
typedef struct heartBeat
{
	int fd;		//客户端连接fd
	int time;	//客户端最后一次操作时间，有操作就加一
}HEARTBEAT;

//EpollServer类：接收N个客户端连接fd和客户端发来的业务包（业务包发给线程池）
class EpollServer
{
public:
	EpollServer(unsigned short port);
	~EpollServer();

	void epollControl(int op, int fd);	//epoll事件队列添加or删除事件
	void epollWork();

	void updateOnlineClient(int fd);			//更新在线用户数据，用于心跳检测
	void deleteClient(int fd);					//删除下线用户数据

	static void* epollThreadFun(void* arg);		//epoll线程执行函数：执行接收后置服务器返回包业务
	static void* heartBeatThreadFun(void* arg);	//心跳检测执行函数：客户端长期不发消息，断开客户端fd
private:
	int res;

	int epollfd;				//epoll文件描述符
	int epollWaitNum;			//已经发生的事件个数

	int socketfd;				//服务器socketfd
	int acceptfd;				//客户端qcceptfd

	char packet[PACKET_SIZE];	//接收客户端发来的业务包

	struct epoll_event epollEvent;					//epoll事件结构体
	struct epoll_event epollEventArray[EPOLL_SIZE];	//epoll事件就序数组：保存已经触发事件的fd

	map<string, HEARTBEAT> onlineClients;			//在线用户map string：用户；HEARTBEAT：心跳检测结构体
	map<string, HEARTBEAT> onlineClients_heartBeat;	//心跳检测map，用于比较客户端是否存活

	TCPServer* TCP;									//epoll类包含tcp类

	ThreadPool* threadPool;							//epoll类 包含 线程池类

	IPC* ipc;										//epoll类 包含 共享内存类
};

