#include "IPC.h"

IPC::IPC()
{
	//初始化
	this->shmArr[BLOCK_NUM] = { 0 };	//共享内存索引区 0—对应下标数据区空闲 1—对应下标数据区有数据
	this->sndIndex = 0;
	this->rsvIndex = 0;
	this->shmAddr = NULL;
	bzero(this->sndPacket, sizeof(this->sndPacket));
	bzero(this->rsvPacket, sizeof(this->rsvPacket));

	//创建共享内存
	this->shmID = shmget((key_t)1004, sizeof(shmArr) + sizeof(PACKET_SIZE) * BLOCK_NUM, IPC_CREAT | 0777);
	if (0 > shmID)
	{
		perror("shmget error");
	}
	else
	{
		cout << "共享内存创建完成" << endl;
	}

	//创建消息队列
	this->msgID = msgget((key_t)1005, IPC_CREAT | 0777);
	if (0 > msgID)
	{
		perror("msgget error");
	}
	else
	{
		cout << "消息队列创建完成" << endl;
	}

	//创建信号量
	this->semID = semCreate((key_t)1006, BLOCK_NUM);	//共享内存数据区个数=信号量个数
	for (int i = 0; i < BLOCK_NUM; i++)
	{
		semSetVal(i, 1);						//信号量初始化
	}
	cout << "信号量创建完成" << endl;
}

IPC::~IPC()
{
	delete this->shmAddr;
	delete this->shmArr;
	delete this->sndPacket;
}

int IPC::semCreate(key_t key, int numSems)
{
	//创建or访问信号量
	//key_t：信号量key，若不存在则创建，若存在则访问；numSems：信号量数组长度
	int res = semget(key, numSems, IPC_CREAT | 0777);
	if (0 > res)
	{
		perror("semget error");
	}
	return res;
}

void IPC::semSetVal(int semsndIndex, int value)
{
	union semun arg;
	arg.val = value;

	//信号量初始化
	//semID：哪个信号量；semsndIndex：下标；value：具体数值
	int res = semctl(this->semID, semsndIndex, SETVAL, arg);
	if (0 > res)
	{
		perror("semctl error");
	}
}

void IPC::semP(int semsndIndex)
{
	struct sembuf buf = { semsndIndex,-1,SEM_UNDO };

	int res = semop(this->semID, &buf, 1);
	if (0 > res)
	{
		perror("semop p error");
	}
}

void IPC::semV(int semsndIndex)
{
	struct sembuf buf = { semsndIndex,1,SEM_UNDO };

	int res = semop(this->semID, &buf, 1);
	if (0 > res)
	{
		perror("semop v error");
	}
}

void IPC::setShm(char resbuf[])
{
	cout << "------------------------------------------------" << endl;

	//接收业务包
	bzero(this->sndPacket, sizeof(this->sndPacket));
	memcpy(this->sndPacket, resbuf, sizeof(this->sndPacket));

	//共享内存操作
	this->shmAddr = shmat(this->shmID, NULL, 0);			//连接共享内存

	memcpy(this->shmArr, this->shmAddr, sizeof(this->shmArr));	//获取当前共享内存索引区信息
	for (int i = 0; i < BLOCK_NUM; i++)
	{
		if (0 == this->shmArr[i])
		{
			this->sndIndex = i;	//若索引区对应数据域空闲，记录当前索引区下标
			break;
		}
	}
	cout << "共享内存当前操作索引区下标 sndIndex = " << sndIndex << endl;

	//对应信号量加锁
	semP(this->sndIndex);

		//数据写入共享内存
		memcpy(shmAddr + sizeof(shmArr) + PACKET_SIZE*sndIndex, &this->sndPacket, PACKET_SIZE);	//操作数据区：首地址 + 索引区总长度 + 数据区*下标
		shmArr[sndIndex] = 1;
		memcpy(shmAddr + sizeof(int)*sndIndex, &shmArr[sndIndex], sizeof(int));					//操作索引区：首地址 + 一个索引大小*下标
		cout << "前置服务器新包丢进共享内存 " << endl;

		/*
		//测试：直接读出
		char data[sndPacket_SIZE] = { 0 };
		memcpy(&data, this->shmAddr + sizeof(shmArr) + sndPacket_SIZE*this->sndIndex, sndPacket_SIZE);

		REBUILD_HEAD head = { 0 };
		memcpy(&head, data, sizeof(REBUILD_HEAD));
		cout << "-----------------------------------------------" << endl;
		cout << "测试读取业务包头" << endl;
		cout << "head.id = " << head.id << endl;
		cout << "head.length = " << head.length << endl;
		cout << "head.type = " << head.type << endl;
		cout << "head.CRC = " << head.CRC << endl;
		cout << "head.acceptfd = " << head.acceptfd << endl;
		LOGIN body = { 0 };
		memcpy(&body, data + sizeof(REBUILD_HEAD), head.length);
		cout << "测试读取业务包体" << endl;
		cout << "body.id = " << body.id << endl;
		cout << "body.name = " << body.name << endl;
		cout << "body.password = " << body.password << endl;
		cout << "-----------------------------------------------" << endl;*/

		//共享内存断开链接
		shmdt(this->shmAddr);

	//对应信号量解锁
	semV(this->sndIndex);

	//发送消息至消息队列，mtype=1：提示后端服务器读取数据
	MSGBUF buf = { 0 };
	buf.mtype = 1;
	sprintf(buf.mtext, "%d", sndIndex);			//消息初始化

	if (-1 == msgsnd(msgID, &buf, sizeof(buf.mtext), 0))	//消息发送
	{
		perror("msgsnd error");
	}
	cout << "消息发送成功" << endl;

	//数据初始化
	bzero(&shmArr[sndIndex], sizeof(int));
	this->sndIndex = 0;

	cout << "------------------------------------------------" << endl;
}

void IPC::getShm()
{
	cout << "------------------------------------------------" << endl;

	//接收后置服务器发来的消息 mtype=2
	this->buf = { 0 };
	if (-1 == msgrcv(this->msgID, &buf, sizeof(buf.mtext), 2, 0))	//接收消息
	{
		perror("msgrcv error");
	}
	this->rsvIndex = atoi(buf.mtext);
	cout << "接收消息成功" << endl;
	cout << "收到共享内存索引区下标 rsvIndex = " << this->rsvIndex << endl;

	//共享内存操作
	shmAddr = shmat(shmID, NULL, 0);	//连接共享内存

	memcpy(&this->shmArr[rsvIndex], this->shmAddr + sizeof(int) * this->rsvIndex, sizeof(int));				//读索引区：首地址 + 一个索引*下标
	if (1 == this->shmArr[rsvIndex])					//若索引区对应数据区内有数据
	{
		semP(this->rsvIndex);		//对应信号量加锁

			//从共享内存中读取数据
			//读数据区：首地址 + 整个索引区长度 + 数据块*下标
			bzero(&rsvPacket, PACKET_SIZE);		//数据初始化
			memcpy(&this->rsvPacket, this->shmAddr + sizeof(shmArr) + PACKET_SIZE * this->rsvIndex, PACKET_SIZE);

			//测试：获取的返回包
			/*REBUILD_HEAD head = {0};
			memcpy(&head, this->rsvPacket, sizeof(REBUILD_HEAD));
			cout << "-----------------------------------------------" << endl;
			cout << "测试读取返回包头" << endl;
			cout << "head.id = " << head.id << endl;
			cout << "head.length = " << head.length << endl;
			cout << "head.type = " << head.type << endl;
			cout << "head.CRC = " << head.CRC << endl;
			cout << "head.acceptfd = " << head.acceptfd << endl;
			LOGINBACK body = { 0 };
			memcpy(&body, this->rsvPacket + sizeof(REBUILD_HEAD), head.length);
			cout << "测试读取返回包体" << endl;
			cout << "body.result = " << body.result << endl;
			cout << "body.page = " << body.page << endl;
			cout << "-----------------------------------------------" << endl; */

			//清空读出的数据区数据
			//修改索引区为空闲—0
			memset(this->shmAddr + sizeof(this->shmArr) + PACKET_SIZE * this->rsvIndex, 0, PACKET_SIZE);
			memset(this->shmAddr + sizeof(int) * this->rsvIndex, 0, sizeof(int));

			//共享内存断开链接
			shmdt(this->shmAddr);

		semV(this->rsvIndex);		//对应信号量解锁

		//数据初始化
		bzero(&buf, sizeof(MSGBUF));
		bzero(&this->shmArr[rsvIndex], sizeof(int));
		this->rsvIndex = 0;
	}

	cout << "------------------------------------------------" << endl;
}

char* IPC::getRsvPacket()
{
	return this->rsvPacket;
}
