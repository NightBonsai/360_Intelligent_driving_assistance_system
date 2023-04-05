#pragma once

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include "protocol.h"

#define BLOCK_NUM 100		//共享内存数据块数量
#define PACKET_SIZE 2048	//业务包大小&共享内存一个数据区大小

using namespace std;

//信号量结构体
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds* buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short* array;  /* Array for GETALL, SETALL */
	struct seminfo* __buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};

//消息结构体
typedef struct messageBuf
{
	long mtype;       /* message type, must be > 0 */
	char mtext[2];    /* message data */
}MSGBUF;

//实时日志结构体
typedef struct curLog
{
	int businessNum;	//用户执行业务数
	int videoListNum;	//视频列表获取数
	int videoRecordNum;	//视频播放记录业务数
}CURLOG;

//前后置服务器通信类
class IPC
{
public:
	IPC();
	~IPC();

	//创建/访问信号量
	int semCreate(key_t key, int numSems);

	//信号量初始化
	void semSetVal(int semIndex, int value);

	void semP(int semIndex);	//信号量P操作	-1加锁
	void semV(int semIndex);	//信号量V操作	+1解锁

	//访问共享内存
	void setShm(char resbuf[]);	//业务包丢进共享内存
	void getShm();				//从共享内存获取业务返回包

	map<string, CURLOG> currentLog;	//存储实时日志信息

	//公共接口
	char* getRsvPacket();
private:
	int shmID, msgID, semID;			//共享内存，消息队列，信号量ID
	
	int shmArr[BLOCK_NUM];				//共享内存索引区	0—对应下标数据区空闲 1—对应下标数据区有数据
	int sndIndex, rsvIndex;				//记录当前索引区下标
	void* shmAddr;						//共享内存地址

	char sndPacket[PACKET_SIZE];		//接收线程池发来的客户端业务包，转发给共享内存
	char rsvPacket[PACKET_SIZE];		//从共享内存接收业务返回包

	MSGBUF buf;							//接收后置服务器发来的消息
};

