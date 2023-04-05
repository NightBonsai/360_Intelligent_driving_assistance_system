#pragma once

#include <queue>
#include <list>
#include <pthread.h>
#include <algorithm>
#include <iostream>
#include "BaseTask.h"

#define MIN_NUM 10

using namespace std;

//线程池类：接收客户端发来的业务包，将包丢给共享内存
class ThreadPool
{
public:
	ThreadPool(const int num = MIN_NUM);
	~ThreadPool();

	void threadInit();				//线程初始化

	void addTask(BaseTask* task);	//任务队列添加任务
	BaseTask* popTask();			//任务队列取出任务

	void move2Idle(pthread_t id);	//线程移至空闲链表
	void move2Busy(pthread_t id);	//线程移至忙碌链表

	bool queueIsEmpty();			//判断任务队列是否为空

	void lock();					//互斥量关锁
	void unlock();					//互斥量解锁

	void wait();					//线程条件变量阻塞	
	void wakeUp();					//线程条件变量唤醒

	static void* threadFun(void* arg);	//线程执行函数——静态：在预编译时就存在，确保线程执行函数在生成类对象前就存在
private:
	int threadMinNum;	//线程最小数量
	int threadMaxNum;	//线程最大数量

	queue<BaseTask*> taskQueue;		//任务队列
	list<pthread_t> idleList;		//空闲链表
	list<pthread_t> busyList;		//忙碌链表

	pthread_mutex_t mutexLock;		//互斥量锁——解决线程争抢数据问题
	pthread_cond_t cond;			//线程条件变量——解决线程阻塞/唤醒问题
};
