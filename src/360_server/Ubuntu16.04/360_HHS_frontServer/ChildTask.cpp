#include "ChildTask.h"

ChildTask::ChildTask(char* data,int type) :BaseTask(data)
{
	this->index = 0;
	this->businessType = type;
	this->rsvHead = { 0 };
}

ChildTask::~ChildTask()
{
	delete this->ipc;
}

void ChildTask::work()
{
	switch (this->businessType)
	{
	case SEND_TO_BACK:
		this->toBackServer();
		break;
	case GET_FROM_BACK:
		this->toClient();
		break;
	}
}

void ChildTask::toBackServer()
{
	//1.接收客户端发来的业务包
	cout << "前置服务器业务执行中.............." << this->data << endl;

	//读头读体
	HEAD oldHead = { 0 };
	char headBuf[PACKET_SIZE] = { 0 };
	char bodyBuf[PACKET_SIZE] = { 0 };

	memcpy(headBuf, this->data, sizeof(HEAD));
	memcpy(&oldHead, headBuf, sizeof(HEAD));
	cout << "--------------------------------------------------------" << endl;
	cout << "前置服务器读取业务包头" << endl;
	cout << "oldHead.id = " << oldHead.id << endl;
	cout << "oldHead.type = " << oldHead.type << endl;
	cout << "oldHead.length = " << oldHead.length << endl;
	cout << "oldHead.CRC = " << oldHead.CRC << endl;

	memcpy(bodyBuf, this->data + sizeof(HEAD), oldHead.length);
	cout << "前置服务器读取业务包体" << endl;

	//CRC判断
	size_t length = strlen(bodyBuf);						//计算体长
	uint32_t crc = CRC::calculate_crc32(bodyBuf, length);	//计算CRC校验码
	cout << "uint32_t数据类型长度 = " << sizeof(uint32_t) << endl;
	cout << "crc校验码计算 crc = " << crc << endl;
	if (crc == oldHead.CRC)
	{
		//业务包发给后置服务器
		//重新封装头
		REBUILD_HEAD newHead = { 0 };
		memcpy(&newHead.id, &oldHead.id, sizeof(oldHead.id));
		newHead.type = oldHead.type;
		newHead.length = oldHead.length;
		newHead.CRC = oldHead.CRC;
		newHead.acceptfd = this->workfd;
		cout << "重新封装头" << endl;
		cout << "newHead.id = " << newHead.id << endl;
		cout << "newHead.type = " << newHead.type << endl;
		cout << "newHead.length = " << newHead.length << endl;
		cout << "newHead.CRC = " << newHead.CRC << endl;
		cout << "newHead.acceptfd = " << newHead.acceptfd << endl;

		//重新封装包
		char newPacket[PACKET_SIZE] = { 0 };
		memcpy(newPacket, &newHead, sizeof(REBUILD_HEAD));															//添加协议头
		memcpy(newPacket + sizeof(REBUILD_HEAD), bodyBuf, newHead.length);											//添加协议体
		cout << "前置服务器新包封装完成 " << endl;

		//新包丢进共享内存
		ipc->setShm(newPacket);

		//实时日志输出
		this->currentLogOutput(newHead, newPacket);
	}
	else
	{
		//封装返回包发给客户端
		//封装crc校验失败返回包头
		//封装crc校验失败返回包体
		BACK_PACK crcBody = { 0 };
		crcBody.result = -1;		//失败

		HEAD crcHead = { 0 };
		memcpy(&crcHead.id, oldHead.id, sizeof(oldHead.id));
		crcHead.type = oldHead.type;
		crcHead.length = sizeof(crcBody);
		crcHead.CRC = oldHead.CRC;

		//封装crc校验失败返回包
		char crcPacket[PACKET_SIZE] = { 0 };
		memcpy(crcPacket, &crcHead, sizeof(HEAD));							//添加协议头
		memcpy(crcPacket + sizeof(HEAD), &crcBody, crcHead.length);			//添加协议体
		cout << "前置服务器crc校验失败返回包封装完成 " << endl;

		//返回包发给客户端
		int writeRes = write(this->workfd, crcPacket, sizeof(crcPacket));
		if (0 < writeRes)
		{
			cout << "前置服务器crc校验失败返回包发送成功" << endl;
		}
		else
		{
			cout << "前置服务器crc校验失败返回包发送失败" << endl;
		}
	}

	cout << "------------------------------------------------" << endl;
}

void ChildTask::toClient()
{
	//2.等待后置服务器返回业务返回包
	cout << "前置服务器返回业务执行中.............." << endl;

	//读头读体
	this->rsvHead = { 0 };						//数据初始化
	char returnBodyBuf[PACKET_SIZE] = { 0 };

	memcpy(&this->rsvHead, this->ipc->getRsvPacket(), sizeof(REBUILD_HEAD));
	cout << "------------------------------------------------" << endl;
	cout << "前置服务器读取返回业务包头" << endl;
	cout << "rsvHead.id = " << rsvHead.id << endl;
	cout << "rsvHead.length = " << rsvHead.length << endl;
	cout << "rsvHead.type = " << rsvHead.type << endl;
	cout << "rsvHead.CRC = " << rsvHead.CRC << endl;
	cout << "rsvHead.acceptfd = " << rsvHead.acceptfd << endl;

	memcpy(returnBodyBuf, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "前置服务器读取返回业务包体" << endl;

	//测试打印
	/*
	if (this->rsvHead.type == GET_IMAGE_LIST)
	{
		GET_PHOTOS_INFO photoList;
		memcpy(&photoList, returnBodyBuf, this->rsvHead.length);
		cout << "photoList数据打印" << endl;
		cout << photoList.allPhoto << endl;
		cout << photoList.page << endl;
		for (int i = 1; i <= photoList.allPhoto; i++)
		{
			cout << photoList.photoArr[i - 1].photoPath << endl;
			cout << photoList.photoArr[i - 1].traitPhotoTime << endl;
		}
	}*/

	//重新封装返回包头
	HEAD returnHead = { 0 };
	memcpy(&returnHead.id, &this->rsvHead.id, sizeof(this->rsvHead.id));
	returnHead.type = this->rsvHead.type;
	returnHead.length = this->rsvHead.length;
	returnHead.CRC = this->rsvHead.CRC;
	cout << "重新封装头" << endl;
	cout << "returnHead.id = " << returnHead.id << endl;
	cout << "returnHead.type = " << returnHead.type << endl;
	cout << "returnHead.length = " << returnHead.length << endl;
	cout << "returnHead.CRC = " << returnHead.CRC << endl;
	cout << "acceptfd = " << this->rsvHead.acceptfd << endl;

	//重新封装包
	char newReturnPacket[PACKET_SIZE] = { 0 };
	memcpy(newReturnPacket, &returnHead, sizeof(HEAD));							//添加协议头
	memcpy(newReturnPacket + sizeof(HEAD), returnBodyBuf, returnHead.length);	//添加协议体
	cout << "前置服务器新返回包封装完成 " << endl;

	//返回包发给客户端
	int writeRes = write(this->rsvHead.acceptfd, newReturnPacket, sizeof(newReturnPacket));
	if (0 < writeRes)
	{
		cout << "前置服务器新返回包发送成功" << endl;
	}
	else
	{
		cout << "前置服务器新返回包发送失败" << endl;
	}

	cout << "------------------------------------------------" << endl;
}

void ChildTask::currentLogOutput(const REBUILD_HEAD newHead,const char newPacket[])
{
	//打印日志
	cout << "----------------------------实时日志----------------------------" << endl;
	map<string, CURLOG>::iterator map_iter = this->ipc->currentLog.find(newHead.id);
	if (map_iter != this->ipc->currentLog.end())
	{
		//当前用户Id存在
		map_iter->second.businessNum++;
		if (newHead.type == USER_LOGIN)
		{
			map_iter->second.videoListNum++;
		}
	}
	else
	{
		//当前用户Id不存在
		//准备日志结构体
		CURLOG log;
		log.businessNum = 1;
		if (newHead.type == USER_LOGIN)
		{
			log.videoListNum = 1;
		}
		else
		{
			log.videoListNum = 0;
		}
		log.videoRecordNum = 0;

		this->ipc->currentLog[newHead.id] = log;	//写入日志
	}
	char packet[PACKET_SIZE] = { 0 };
	memcpy(packet, newPacket, sizeof(packet));
	cout << "用户账号 : " << newHead.id << endl;
	cout << "接收数据包大小 : " << sizeof(this->data) << endl;
	cout << "发送数据包大小 : " << sizeof(packet) << endl;
	cout << "用户登录业务数 : " << this->ipc->currentLog.size() << endl;
	cout << "用户登录业务类型 : " << newHead.type << endl;
	cout << "视频业务有用户获取视频列表数 : " << map_iter->second.videoListNum << endl;
	cout << "视频业务有用户视频播放记录业务数 : " << map_iter->second.videoRecordNum << endl;

	cout << "------------------------------------------------" << endl;
}
