#include "BaseTask.h"

BaseTask::BaseTask(char* data)
{
	this->ipc = NULL;

	bzero(this->data, sizeof(this->data));
	mempcpy(this->data, data, sizeof(this->data));
}

BaseTask::~BaseTask()
{
	delete this->data;
}

void BaseTask::setWorkfd(int val)
{
	this->workfd = val;
}

void BaseTask::setIPC(IPC* ipc)
{
	this->ipc = ipc;
}
