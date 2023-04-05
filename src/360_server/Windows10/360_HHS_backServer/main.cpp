#include <iostream>
#include "ThreadPool.h"
#include "ChildTask.h"
#include "IPC.h"

using namespace std;

int main()
{
	ThreadPool* threadPool = new ThreadPool();
	threadPool->threadInit();

	IPC* ipc = new IPC();

	while (true)
	{
		//等待前置业务包消息
		ipc->getShm();

		//创建从前置接收业务包任务
		BaseTask* task = new ChildTask("");

		//任务设置共享内存
		task->setIPC(ipc);

		//线程池添加子任务
		threadPool->addTask(task);
	}

	return 0;
}