#pragma once

#include "SingleDB.h"
#include <string.h>

//数据库交互单例：执行数据库业务
class DBBusiness
{
public:
	static DBBusiness* getInstance();

	//数据库业务
	int searchUserId(char**& qres, int& row, int& col, const char userId[], const char userPwd[]);	//获取用户ID，判断登录是否成功，返回用户昵称
	int addNewUser(const char userId[], const char userName[], const char userPwd[]);				//用户注册业务
	
	int addNewPhoto(const char userId[],const char imgPath[],const char imgSaveTime[]);				//图片数据上传业务
	int searchPhoto(char**& qres, int& row, int& col, const char userId[], const int pageIndex);	//图片信息检索业务：检索一页数据
	
	int addNewVideo(const char userId[], const char videoPath[], const char videoSaveTime[],		//视频数据上传业务
		const int videoCurFrame, const int videoTotalFrame, const char videoCoverPath[]);
	int searchVideo(char**& qres, int& row, int& col, const char userId[], const int pageIndex);	//视频信息检索业务

	int addNewLog(const char userId[], const char operationTime[],									//新增日志业务
		const char operationRecord[], const char operationPath[]);
	int searchLog(char**& qres, int& row, int& col);												//日志检索：用于导出日志表
private:
	DBBusiness();
	static DBBusiness* dbBusiness;
};

