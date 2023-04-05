#include "ChildTask.h"

ChildTask::ChildTask(char* data) :BaseTask(data)
{
	this->ipc = new IPC();
	
	this->index = 0;
	this->operationPath[80] = { 0 };
	this->rsvHead = { 0 };
}

ChildTask::~ChildTask()
{
	delete this->ipc;
}

void ChildTask::work()
{
	//接收客户端发来的业务包
	cout << "后置服务器业务执行中.............." << this->data << endl;

	//读头
	this->rsvHead = { 0 };														//数据初始化
	memcpy(&this->rsvHead, this->ipc->getRsvPacket(), sizeof(REBUILD_HEAD));
	this->workfd = rsvHead.acceptfd;											//记录当前执行业务客户端fd
	cout << "------------------------------------------------" << endl;
	cout << "后置服务器读取业务包头" << endl;
	cout << "rsvHead.id = " << rsvHead.id << endl;
	cout << "rsvHead.length = " << rsvHead.length << endl;
	cout << "rsvHead.type = " << rsvHead.type << endl;
	cout << "rsvHead.CRC = " << rsvHead.CRC << endl;
	cout << "rsvHead.acceptfd = " << rsvHead.acceptfd << endl;

	//读体
	//根据头信息判断业务类型，执行不同的业务逻辑
	switch (rsvHead.type)
	{
	case USER_LOGIN:
		this->loginBusiness();
		break;
	case USER_REGISTER:
		this->registBusiness();
		break;
	case IMAGE_UP_INFO:
		this->photoUploadBusiness();
		break;
	case PHOTO_ASK_INFO:
		this->photoSearchBusiness();
		break;
	case VIDEO_UP_INFO:
		this->videoUploadBusiness();
		break;
	case VIDEO_ASK_INFO:
		this->videoSearchBusiness();
		break;
	}
	cout << "------------------------------------------------" << endl;
}

void ChildTask::loginBusiness()
{
	cout << "------------客户端登录请求业务------------" << endl;

	LOGIN body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.id = " << body.id << endl;
	cout << "body.password = " << body.password << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	LOGINBACK loginBackPack = { 0 };	//返回包体

	char** qres = NULL;					//必要参数
	int row = 0, col = 0;
	list<string> userIdList;
	int res = DBBusiness::getInstance()->searchUserId(qres, row, col, body.id, body.password);
	if (1 == res)
	{
		cout << "客户端登录成功" << endl;
		loginBackPack.result = 1;	//封装业务返回包体
		loginBackPack.page = 0;
		for (int i = col; i <= row; i++)
		{
			userIdList.push_back(qres[i]);
		}
		memcpy(&loginBackPack.name, qres[1], sizeof(qres[0]));
		cout << "返回用户昵称 loginBackPack.name = " << loginBackPack.name << endl;

		//获取视频数据
		char** videoQres = NULL;					//必要参数
		int videoRow = 0, videoCol = 0;
		int res = DBBusiness::getInstance()->searchVideo(videoQres, videoRow, videoCol, body.id, 0);
		if (res > 0)
		{
			cout << "登录视频数据获取成功" << endl;

			//计算必要数据信息
			int videoNum = res - 1;				//视频总个数
			int pageNum = 0;					//视频总页数
			if (videoNum % 6 == 0)
			{
				pageNum = videoNum / 6;
			}
			else
			{
				pageNum = (videoNum / 6) + 1;
			}
			loginBackPack.page = pageNum;		//封装返回包体

			cout << "用户视频数据信息" << endl;
			cout << "视频总个数 videoNum = " << videoNum << endl;
			cout << "视频总页数 pageNum = " << pageNum << endl;
			cout << "获取的视频数据" << endl;
			for (int i = 1; i <= videoRow; i++)
			{
				cout << videoQres[i * videoCol] << " " << videoQres[i * videoCol + 1] << " " << videoQres[i * videoCol + 2] << " " << videoQres[i * videoCol + 3]
					<< " " << videoQres[i * videoCol + 4] << " " << videoQres[i * videoCol + 5] << " " << videoQres[i * videoCol + 6] << endl;
				
				//视频数据加入返回包体
				memcpy(loginBackPack.videoArr[i - 1].videoPath, videoQres[i * videoCol + 1], sizeof(loginBackPack.videoArr[i - 1].videoPath));
				memcpy(loginBackPack.videoArr[i - 1].videoTime, videoQres[i * videoCol + 2], sizeof(loginBackPack.videoArr[i - 1].videoTime));
				loginBackPack.videoArr[i - 1].curFrame = atoi(videoQres[i * videoCol + 3]);
				loginBackPack.videoArr[i - 1].totalFrame = atoi(videoQres[i * videoCol + 4]);
				memcpy(loginBackPack.videoArr[i - 1].videoCoverPath, videoQres[i * videoCol + 5], sizeof(loginBackPack.videoArr[i - 1].videoCoverPath));
			}
		}
		else
		{
			cout << "登录视频数据获取失败" << endl;
		}
	}
	else
	{
		cout << "客户端登录失败" << endl;
		loginBackPack.result = -1;	//封装业务返回包体
		loginBackPack.page = 0;
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	memcpy(sndHead.id, body.id, sizeof(body.id));
	sndHead.length = sizeof(loginBackPack);
	sndHead.type = USERLOGIN_BACK;
	sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &loginBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	//写日志
	char operationRecord[] = "客户端登录请求业务";
	char operationPath[] = "null";
	this->addLogBusiness(operationRecord, operationPath);

	cout << "------------------------------------------------" << endl;
}

void ChildTask::registBusiness()
{
	cout << "------------客户端注册请求业务------------" << endl;

	REGISTER body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.id = " << body.id << endl;
	cout << "body.name = " << body.name << endl;
	cout << "body.password = " << body.password << endl;
	cout << "------------------------------------------------" << endl;

	//访问数据库执行对应业务
	BACK_PACK registBackPack = { 0 };

	int res = DBBusiness::getInstance()->addNewUser(body.id, body.name, body.password);
	if (1 == res)
	{
		cout << "客户端注册成功" << endl;

		//创建新用户图片保存路径
		char writePath[100] = { 0 };
		sprintf(writePath, "userInfo/%s/photo", this->rsvHead.id);
		cout << "新用户图片保存路径 writePath = " << writePath << endl;

		this->createDirs(writePath);

		registBackPack.result = 1;	//封装业务返回包体
	}
	else if(0 == res)
	{
		cout << "客户端注册失败，用户Id已存在" << endl;
		registBackPack.result = 0;	//封装业务返回包体
	}
	else if (-1 == res)
	{
		cout << "客户端注册失败，sql语句执行失败" << endl;
		registBackPack.result = -1;	//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	memcpy(sndHead.id, body.id, sizeof(body.id));
	sndHead.length = sizeof(registBackPack);
	sndHead.type = BACK_BACK;
	sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &registBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	//写日志
	char operationRecord[] = "客户端注册请求业务";
	char operationPath[] = "null";
	this->addLogBusiness(operationRecord, operationPath);

	cout << "------------------------------------------------" << endl;
}

void ChildTask::photoUploadBusiness()
{
	cout << "------------客户端图片数据上传业务------------" << endl;

	PHOTO_INFO body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.photoPath = " << body.photoPath << endl;
	cout << "body.p_fileName = " << body.p_fileName << endl;
	cout << "body.p_fileSize = " << body.p_fileSize << endl;
	cout << "body.p_filePageLen = " << body.p_filePageLen << endl;
	cout << "body.p_filePageNum = " << body.p_filePageNum << endl;
	cout << "body.photoIndex = " << body.p_filePageIndex << endl;
	cout << "------------------------------------------------" << endl;

	//图片分包写入图片容器
	map<string, list<PHOTO_INFO> >::iterator map_iter;
	map_iter = this->ipc->photoInfo.find(body.p_fileName);	//指向对应键值对
	if (map_iter != this->ipc->photoInfo.end())
	{
		cout << "文件名存在，新分包上传" << endl;
		
		map_iter->second.push_back(body);	//分包写入值
		
		//判断图片分包是否收齐
		cout << "图片名 : " << map_iter->first << endl;
		cout << "收到图片分包数 : " << map_iter->second.size() << endl;
		cout << "发送图片分包总数 : " << body.p_filePageNum << endl;
		
		//----------------------------------------测试---------------------------------------------
		/*cout << "接收到的图片分包 : " << endl;
		list<PHOTO_INFO>::iterator list_iter;		//指向对应图片分包list
		for (list_iter = map_iter->second.begin(); list_iter != map_iter->second.end(); list_iter++)
		{
			cout << list_iter->p_filePageIndex << endl;
		}*/

		if (map_iter->second.size() == body.p_filePageNum)	
		{
			cout << "图片分包收齐，进行排序拼接" << endl;

			//图片分包排序
			map_iter->second.sort(IPC::listCompare);	

			//拼接保存路径
			char writePath[100] = { 0 };
			sprintf(writePath, "userInfo/%s/photo/%s",
				this->rsvHead.id, map_iter->first.c_str());
			memcpy(this->operationPath, writePath, sizeof(this->operationPath));	//记录当前本地文件操作路径
			cout << "图片保存路径 writePath = " << writePath << endl;
			cout << "operationPath = " << operationPath << endl;

			//打开保存路径（文件后添加）
			umask(0);															//设置权限掩码
			int wfd = open(writePath, O_CREAT | O_WRONLY | O_APPEND, 0777);
			if (wfd < 0)
			{
				perror("图片保存路径打开失败");
			}

			//进行图片拼接
			list<PHOTO_INFO>::iterator list_iter;		//指向对应图片分包list
			for (list_iter = map_iter->second.begin(); list_iter != map_iter->second.end(); list_iter++)
			{
				write(wfd,list_iter->p_filePageContext,list_iter->p_filePageLen);	//写入文件

				cout << "图片分包写入成功 photoName = " << map_iter->first << " p_filePageIndex = " << list_iter->p_filePageIndex << endl;
			}
			close(wfd);		//关闭写入文件描述符

			//判断图片拼接是否有误
			BACK_PACK photoUpBackPack = { 0 };					//返回包体
			struct stat statbuf;
			stat(writePath, &statbuf);							//获取拼合完成的图片大小

			int size = statbuf.st_size;
			int realSize = body.p_fileSize;
			cout << "拼合完成图片大小 size = " << size << endl;
			cout << "上传图片大小 body.p_fileSize = " << body.p_fileSize << endl;
			if (size == realSize)							
			{
				//判断图片拼接是否有误
				map_iter->second.clear();	//清空图片分包list

				//访问数据库执行对应业务
				char timeBuf[80] = { 0 };
				this->getTime(timeBuf);
				cout << "当前时间 : " << timeBuf << endl;	//获取当前时间
				int res = DBBusiness::getInstance()->addNewPhoto(this->rsvHead.id, body.photoPath, timeBuf);
				if (-1 == res)
				{
					cout << "图片上传失败，sql语句执行失败" << endl;
					photoUpBackPack.result = -1;	//封装业务返回包体
				}
				else
				{
					cout << "图片上传成功" << endl;
					photoUpBackPack.result = 1;		//封装业务返回包体
				}
			}
			else
			{
				//图片拼接有误 
				map_iter->second.clear();		//清空图片分包list
				
				photoUpBackPack.result = -1;	//封装业务返回包体
			}

			//封装业务返回包
			REBUILD_HEAD sndHead = { 0 };
			memcpy(sndHead.id, this->rsvHead.id, sizeof(this->rsvHead.id));
			sndHead.length = sizeof(photoUpBackPack);
			sndHead.type = PHOTO_BACK;
			sndHead.CRC = 1;
			sndHead.acceptfd = this->workfd;

			char sndPacket[PACKET_SIZE] = { 0 };
			memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
			memcpy(sndPacket + sizeof(REBUILD_HEAD), &photoUpBackPack, sndHead.length);	//添加体

			//返回包丢进共享内存
			this->ipc->setShm(sndPacket);

			//写日志
			char operationRecord[] = "客户端图片数据上传业务";
			cout << "operationRecord = " << operationRecord << endl;
			cout << "operationPath = " << this->operationPath << endl;
			cout << "userId = " << this->rsvHead.id << endl;
			this->addLogBusiness(operationRecord, this->operationPath);
		}
	}
	else
	{
		//若文件名不存在
		this->ipc->photoInfo[body.p_fileName].push_back(body);	//新建键值对，分包写入值
		cout << "文件名不存在，新图片上传" << endl;
		cout << "图片 p_fileName = " << body.p_fileName << endl;
	}

	cout << "------------------------------------------------" << endl;
}

void ChildTask::photoSearchBusiness()
{
	cout << "------------客户端图片信息检索业务------------" << endl;

	PHOTO_ASK body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.photo_index = " << body.photo_index << endl;

	//数据库交互，获取当前用户图片
	GET_PHOTOS_INFO photoList = { 0 };		//返回包体
	char** qres = nullptr;
	int row = 0, col = 0;
	int res = DBBusiness::getInstance()->searchPhoto(qres, row, col, this->rsvHead.id, body.photo_index - 1);
	if (res > 0)
	{
		cout << "图片信息检索成功" << endl;
		cout << "row = " << row << " col = " << col << endl;

		//计算必要数据
		int photoNum = res - 1;				//图片总个数
		int pageNum = 0;					//图片总页数
		if (photoNum % 6 == 0)
		{
			pageNum = photoNum / 6;
		}
		else
		{
			pageNum = (photoNum / 6) + 1;
		}
		photoList.page = pageNum;				//封装返回包体
		photoList.allPhoto = row - 1;

		cout << "用户图片数据信息" << endl;
		cout << "图片总个数 photoNum = " << photoNum << endl;
		cout << "图片总页数 pageNum = " << pageNum << endl;
		cout << "当前页图片个数 photoList.allPhoto = " << photoList.allPhoto << endl;
		cout << "获取数据打印" << endl;
		for (int i = 1; i <= row; i++)
		{
			//		图片Id					图片存储路径				图片存储时间				用户Id
			cout << qres[i * col] << " " << qres[i * col + 1] << " " << qres[i * col + 2] << " " << qres[i * col + 3] << endl;

			//记录当前操作的图片存储路径
			memcpy(this->operationPath, qres[i * col + 1], sizeof(this->operationPath));

			//图片数据加入返回包体
			memcpy(photoList.photoArr[i - 1].photoPath, qres[i * col + 1], sizeof(photoList.photoArr[i - 1].photoPath));
			memcpy(photoList.photoArr[i - 1].traitPhotoTime, qres[i * col + 2], sizeof(photoList.photoArr[i - 1].traitPhotoTime));
		}
	}
	else if (res == 0)
	{
		cout << "图片信息检索失败，未找到对应图片信息" << endl;
	}
	else
	{
		cout << "图片信息检索失败，sql语句执行失败" << endl;
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	memcpy(sndHead.id, this->rsvHead.id, sizeof(this->rsvHead.id));
	sndHead.length = sizeof(photoList);
	sndHead.type = GET_IMAGE_LIST;
	sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));						//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &photoList, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	//写日志
	char operationRecord[] = "客户端图片信息检索业务";
	this->addLogBusiness(operationRecord, this->operationPath);

	cout << "------------------------------------------------" << endl;
}

void ChildTask::videoUploadBusiness()
{
	cout << "------------客户端视频数据上传业务------------" << endl;

	VIDEO_INFO body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.videoPath = " << body.videoPath << endl;
	cout << "body.videoCoverPath = " << body.videoCoverPath << endl;
	cout << "body.totalFrame = " << body.totalFrame << endl;
	cout << "body.curFrame = " << body.curFrame << endl;

	//获取当前时间
	char timeBuf[80] = { 0 };
	this->getTime(timeBuf);

	//访问数据库执行对应业务
	BACK_PACK videoBackPack = { 0 };

	int res = DBBusiness::getInstance()->addNewVideo(this->rsvHead.id, body.videoPath, timeBuf,
		body.curFrame, body.totalFrame, body.videoCoverPath);
	if (1 == res)
	{
		cout << "视频信息上传成功" << endl;
		videoBackPack.result = 1;			//封装业务返回包体
	}
	else
	{
		cout << "视频信息上传失败" << endl;
		videoBackPack.result = -1;			//封装业务返回包体
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	memcpy(sndHead.id, this->rsvHead.id, sizeof(this->rsvHead.id));
	sndHead.length = sizeof(videoBackPack);
	sndHead.type = VIDEO_BACK;
	sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));							//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &videoBackPack, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	//写日志
	char operationRecord[] = "客户端视频数据上传业务";
	char operationPath[] = "null";
	this->addLogBusiness(operationRecord, operationPath);

	cout << "------------------------------------------------" << endl;
}

void ChildTask::videoSearchBusiness()
{
	cout << "------------客户端视频信息检索业务------------" << endl;

	VIDEO_ASK body = { 0 };
	memcpy(&body, this->ipc->getRsvPacket() + sizeof(REBUILD_HEAD), this->rsvHead.length);
	cout << "后置服务器读取业务包体" << endl;
	cout << "body.video_index = " << body.video_index << endl;

	//数据库交互，获取当前用户视频
	GET_VIDEOS_INFO videoList = { 0 };		//返回包体
	char** qres = nullptr;
	int row = 0, col = 0;
	int res = DBBusiness::getInstance()->searchVideo(qres, row, col, this->rsvHead.id, body.video_index - 1);
	if (res > 0)
	{
		cout << "视频信息检索成功" << endl;
		cout << "row = " << row << " col = " << col << endl;

		//计算必要数据
		int videoNum = res - 1;				//视频总个数
		int pageNum = 0;					//视频总页数
		if (videoNum % 6 == 0)
		{
			pageNum = videoNum / 6;
		}
		else
		{
			pageNum = (videoNum / 6) + 1;
		}
		videoList.page = pageNum;				//封装返回包体
		videoList.allVideo = row - 1;

		cout << "用户视频数据信息" << endl;
		cout << "视频总个数 videoNum = " << videoNum << endl;
		cout << "视频总页数 pageNum = " << pageNum << endl;
		cout << "当前页视频个数 videoList.allVideo = " << videoList.allVideo << endl;
		cout << "获取数据打印" << endl;
		for (int i = 1; i <= row; i++)
		{
			cout << qres[i * col] << " " << qres[i * col + 1] << " " << qres[i * col + 2] << " " << qres[i * col + 3] 
				<< " " << qres[i * col + 4] << " " << qres[i * col + 5] << " " << qres[i * col + 6] << endl;

			//记录当前操作的视频存储路径
			memcpy(this->operationPath, qres[i * col + 1], sizeof(this->operationPath));

			//视频数据加入返回包体
			memcpy(videoList.videoArr[i - 1].videoPath, qres[i * col + 1], sizeof(videoList.videoArr[i - 1].videoPath));
			memcpy(videoList.videoArr[i - 1].videoTime, qres[i * col + 2], sizeof(videoList.videoArr[i - 1].videoTime));
			videoList.videoArr[i - 1].curFrame = atoi(qres[i * col + 3]);
			videoList.videoArr[i - 1].totalFrame = atoi(qres[i * col + 4]);
			memcpy(videoList.videoArr[i - 1].videoCoverPath, qres[i * col + 5], sizeof(videoList.videoArr[i - 1].videoCoverPath));
		}
	}
	else if (res == 0)
	{
		cout << "视频信息检索失败，未找到对应视频信息" << endl;
	}
	else
	{
		cout << "视频信息检索失败，sql语句执行失败" << endl;
	}

	//封装业务返回包
	REBUILD_HEAD sndHead = { 0 };
	memcpy(sndHead.id, this->rsvHead.id, sizeof(this->rsvHead.id));
	sndHead.length = sizeof(videoList);
	sndHead.type = GET_VIDEO_LIST;
	sndHead.CRC = 1;
	sndHead.acceptfd = this->workfd;

	char sndPacket[PACKET_SIZE] = { 0 };
	memcpy(sndPacket, &sndHead, sizeof(REBUILD_HEAD));						//添加头
	memcpy(sndPacket + sizeof(REBUILD_HEAD), &videoList, sndHead.length);	//添加体

	//返回包丢进共享内存
	this->ipc->setShm(sndPacket);

	//写日志
	char operationRecord[] = "客户端视频信息检索业务";
	this->addLogBusiness(operationRecord, this->operationPath);

	cout << "------------------------------------------------" << endl;
}

void ChildTask::addLogBusiness(const char operationRecord[], const char operationPath[])
{
	//获取当前时间
	char buffer[80] = { 0 };

	this->getTime(buffer);
	cout << "后置服务器业务操作时间：" << buffer << endl;
	cout << "operationRecord = " << operationRecord << endl;
	cout << "operationPath = " << operationPath << endl;
	cout << "userId = " << this->rsvHead.id << endl;
	int res = DBBusiness::getInstance()->addNewLog(this->rsvHead.id, buffer, operationRecord, operationPath);
	if (1 == res)
	{
		cout << "日志写入成功" << endl;
	}
	else
	{
		cout << "日志写入失败" << endl;
	}
}

void ChildTask::getTime(char buffer[])
{
	//获取当前时间
	time_t rawtime;		//time_t 能够把系统时间和日期表示为某种整数
	struct tm* info;	//结构类型 tm 把日期和时间以 C 结构的形式保存

	time(&rawtime);										//返回系统的当前日历时间
	info = localtime(&rawtime);							//返回一个指向表示本地时间的 tm 结构的指针
	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);	//格式化日期和时间为指定的格式
}

bool ChildTask::createDirs(const std::string& dirName)
{
	uint32_t beginCmpPath = 0;
	uint32_t endCmpPath = 0;

	std::string fullPath = "";

	printf("path = %s\n", dirName.c_str());

	if ('/' != dirName[0])
	{ //Relative path  
		//get current path
		fullPath = getcwd(nullptr, 0);

		beginCmpPath = fullPath.size();

		printf("current Path: %s\n", fullPath.c_str());
		fullPath = fullPath + "/" + dirName;
	}
	else
	{
		//Absolute path
		fullPath = dirName;
		beginCmpPath = 1;
	}

	if (fullPath[fullPath.size() - 1] != '/')
	{
		fullPath += "/";
	}

	endCmpPath = fullPath.size();


	//create dirs;
	for (uint32_t i = beginCmpPath; i < endCmpPath; i++)
	{
		if ('/' == fullPath[i])
		{
			std::string curPath = fullPath.substr(0, i);
			if (access(curPath.c_str(), F_OK) != 0)
			{
				if (mkdir(curPath.c_str(), S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
				{
					printf("mkdir(%s) failed(%s)\n", curPath.c_str(), strerror(errno));
					return false;
				}
			}
		}
	}

	return true;
}

void ChildTask::setIndex(int val)
{
	this->index = val;
}
