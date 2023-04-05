#include "EpollServer.h"

EpollServer::EpollServer(unsigned short port)
{
	//网络连接
	this->TCP = new TCPServer(port);
	this->TCP->startConnect();

	//初始化
	this->res = 0;
	this->acceptfd = 0;
	this->epollWaitNum = 0;
	this->epollfd = epoll_create(EPOLL_SIZE);						//创建epoll

	this->socketfd = this->TCP->getSocketfd();
	this->epollControl(EPOLL_CTL_ADD, this->socketfd);				//前置条件：将服务器socketfd加入epoll事件数组中
	cout << "网络连接成功，socketfd：" << this->socketfd << endl;

	bzero(this->packet, sizeof(this->packet));

	//共享内存初始化
	this->ipc = new IPC();

	//线程池初始化
	this->threadPool = new ThreadPool();
	this->threadPool->threadInit();

	//创建epoll接收后置服务器返回包线程
	pthread_t pid;
	pthread_create(&pid, NULL, epollThreadFun, this);

	//创建心跳检测业务线程
	pthread_t heartPid;
	pthread_create(&heartPid, NULL, heartBeatThreadFun, this);
}

EpollServer::~EpollServer()
{
	delete this->TCP;
	delete this->epollEventArray;
	delete this->threadPool;
	delete this->packet;
}

void EpollServer::epollControl(int op, int fd)
{
	//epoll对 完成连接的客户端操作
	bzero(&this->epollEvent, sizeof(this->epollEvent));		//初始化

	epollEvent.data.fd = fd;								//连接成功的客户端acceptfd放入结构体
	epollEvent.events = EPOLLIN;							//网络搭建成功socketfd可能会发送的EPOLLIN事件

	epoll_ctl(this->epollfd, op, fd, &this->epollEvent);	//epoll添加or删除事件
}

void EpollServer::epollWork()
{
	while (1)
	{
		cout << "epoll wait.................." << endl;

		//eopll等待事件发生
		//执行成功返回执行的事件数目，失败返回-1，超时返回0
		this->epollWaitNum = epoll_wait(this->epollfd, this->epollEventArray, EPOLL_SIZE, -1);	//epoll_wait阻塞式函数	数组：已经触发的事件fd
		if (-1 == epollWaitNum)
		{
			perror("epoll_wait error");
		}

		//有事件发生： 1.有客户端连接                                ---添加到epoll事件列表
		//             2.客户端发送数据                              ---读取数据
		//             3.客户端下线(正常退出、终端、kill、异常退出） ---从epoll事件链表删除
		for (int i = 0; i < this->epollWaitNum; i++)
		{
			if (epollEventArray[i].data.fd == this->socketfd)							//服务器地址被连接，服务器socket响应：客户端上线
			{
				cout << "有客户端连接" << endl;

				//阻塞函数：等待用户连接
				this->acceptfd = accept(this->socketfd, NULL, NULL);
				cout << "客户端连接成功 acceptfd：" << this->acceptfd << endl;

				//完成连接的客户端放入epoll
				this->epollControl(EPOLL_CTL_ADD, this->acceptfd);
			}
			else if (epollEventArray[i].events & EPOLLIN)								//数组中有事件 且 事件是EPOLLIN：客户端请求业务处理
			{
				cout << "有事件发生，但不是socket，是客户端有操作" << endl;

				//读数据判断是否有数据：有：客户端业务；无：客户端下线
				bzero(this->packet, sizeof(this->packet));
				this->res = read(epollEventArray[i].data.fd, this->packet, sizeof(this->packet));
				if (res > 0)
				{
					cout << epollEventArray[i].data.fd << " 客户端发来数据 " << endl;

					//创建子任务
					BaseTask* task = new ChildTask(this->packet, SEND_TO_BACK);

					task->setWorkfd(epollEventArray[i].data.fd);	//设置客户端acceptfd
					task->setIPC(this->ipc);						//设置共享内存

					//线程池添加任务
					this->threadPool->addTask(task);

					//更新在线用户map
					this->updateOnlineClient(epollEventArray[i].data.fd);
				}
				else
				{
					cout << "客户端下线 acceptfd = " << epollEventArray[i].data.fd << endl;

					//epoll事件数组删除
					this->epollControl(EPOLL_CTL_DEL, epollEventArray[i].data.fd);

					//关闭下线的客户端
					close(epollEventArray[i].data.fd);

					//在线用户数据删除
					this->deleteClient(epollEventArray[i].data.fd);
				}
			}
		}
	}
}

void EpollServer::updateOnlineClient(int fd)
{
	//读取当前业务包头
	HEAD head = { 0 };

	memcpy(&head, this->packet, sizeof(HEAD));
	cout << "----------------------------epoll接收到的用户Id----------------------------" << endl;
	cout << "head.id = " << head.id << endl;

	//检索用户Id账号是否在
	map<string, HEARTBEAT>::iterator map_iter = this->onlineClients.find(head.id);
	map<string, HEARTBEAT>::iterator map_iter2 = this->onlineClients_heartBeat.find(head.id);
	if (map_iter != this->onlineClients.end())	
	{
		//如果用户Id账号在线
		//更新用户操作时间
		map_iter2->second.time++;		//只更新heartBeatMap

		cout << "用户数据更新" << endl;
		cout << "userId = " << map_iter2->first << endl;
		cout << "fd = " << map_iter2->second.fd << endl;
		cout << "time2 = " << map_iter2->second.time << endl;
	}
	else
	{
		//如果用户Id账号不在线
		//map新增数据
		this->onlineClients[head.id].fd = fd;
		this->onlineClients[head.id].time = 0;

		this->onlineClients_heartBeat[head.id].fd = fd;
		this->onlineClients_heartBeat[head.id].time = 1;

		cout << "新用户上线" << endl;
		cout << "userId = " << head.id << endl;
		cout << "fd = " << this->onlineClients[head.id].fd << endl;
		cout << "time1 = " << this->onlineClients[head.id].time << endl;
		cout << "time2 = " << this->onlineClients_heartBeat[head.id].time << endl;
	}

	cout << "----------------------------在线用户数据更新成功----------------------------" << endl;
}

void EpollServer::deleteClient(int fd)
{
	map<string, HEARTBEAT>::iterator map_iter = this->onlineClients.begin();
	map<string, HEARTBEAT>::iterator map_iter2 = this->onlineClients_heartBeat.begin();
	for (; map_iter != this->onlineClients.end() && map_iter2 != this->onlineClients_heartBeat.end(); map_iter++, map_iter2++)
	{
		if (map_iter->second.fd == fd)
		{
			this->onlineClients.erase(map_iter);
		}

		if (map_iter2->second.fd == fd)
		{
			this->onlineClients_heartBeat.erase(map_iter2);
		}
	}

	cout << "在线用户数据删除" << endl;
}

void* EpollServer::epollThreadFun(void* arg)
{
	EpollServer* epoll = (EpollServer*)arg;

	while (true)
	{
		cout << "epoll返回业务执行中.............." << endl;
		
		//等待后置返回消息和返回包
		epoll->ipc->getShm();

		//创建从后置接收返回包任务
		BaseTask* task = new ChildTask("", GET_FROM_BACK);

		//任务设置共享内存
		task->setIPC(epoll->ipc);

		//线程池添加任务
		epoll->threadPool->addTask(task);
	}
}

void* EpollServer::heartBeatThreadFun(void* arg)
{
	EpollServer* epoll = (EpollServer*)arg;

	while (true)
	{
		sleep(HEART_BEAT_TIME);

		cout << "心跳业务执行中.............." << endl;

		//遍历在线用户表
		map<string, HEARTBEAT>::iterator map_iter = epoll->onlineClients.begin();
		map<string, HEARTBEAT>::iterator map_iter2 = epoll->onlineClients_heartBeat.begin();
		for (; map_iter != epoll->onlineClients.end(); map_iter++)
		{
			for (map_iter2 = epoll->onlineClients_heartBeat.begin(); map_iter2 != epoll->onlineClients_heartBeat.end(); map_iter2++)
			{
				if (strcmp(map_iter->first.c_str(), map_iter2->first.c_str()) == 0)
				{
					//若心跳间隔时间后两次操作时间都相同
					if (map_iter2->second.time == map_iter->second.time)
					{
						//客户端已离线，杀死客户端线程
						cout << "客户端异常掉线 fd = " << map_iter2->second.fd << endl;

						//epoll事件数组删除
						epoll->epollControl(EPOLL_CTL_DEL, map_iter2->second.fd);

						//关闭下线的客户端
						close(map_iter2->second.fd);

						//在线用户数据删除
						epoll->deleteClient(map_iter2->second.fd);
					}
					//若心跳间隔时间后两次操作时间不相同
					else
					{
						//更新用户操作时间
						map_iter->second.time = map_iter2->second.time;

						cout << "----------------------------用户数据同步----------------------------" << endl;
						cout << "userId = " << map_iter->first << endl;
						cout << "fd = " << map_iter->second.fd << endl;
						cout << "time1 = " << map_iter->second.time << endl;
					}
				}
			}
		}

	}
}
