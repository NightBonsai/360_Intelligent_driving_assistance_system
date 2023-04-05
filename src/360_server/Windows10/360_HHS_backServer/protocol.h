#pragma once
//----------------------------------------------------------业务类型宏定义-----------------------
//业务的1-8 与 协议体1-8   对应

#define USER_LOGIN 1		//登录（请求）
#define USER_REGISTER 2		//注册（请求）
#define VIDEO_UP_INFO 3		//视频信息上传（请求）
#define IMAGE_UP_INFO 4		//图片和图片信息上传（请求）
#define PHOTO_ASK_INFO 10   //请求获得图片信息
#define VIDEO_ASK_INFO 12   //请求获取视频信息

#define USERLOGIN_BACK 5	//登录返回（反馈）
#define BACK_BACK 6			//反馈包（反馈）         
#define GET_VIDEO_LIST 7	//视频信息列表反馈包（反馈）
#define GET_IMAGE_LIST 8	//图片信息列表反馈包（反馈）
#define PHOTO_BACK 9        //图片是否上传成功（反馈） ---也指向协议体 6
#define VIDEO_BACK 11		//视频是否上传成功（反馈） ---也指向协议体 6
#define CRC_FAIL  13        //CRC验证失败（反馈）       --也指向协议体 6

//----------------------------------------------------------协议头------------------------------------
//客户端→前置服务器
typedef struct head
{
	char id[20];	//用户账号
	int type;		//业务类型
	int length;		//协议体长度
	int CRC;		//CRC循环冗余校验码
}HEAD;

//前置服务器→后置服务器
typedef struct rebuiltHead
{
	char id[20];	//用户账号（id）
	int type;		//业务类型
	int length;		//协议体长度
	int CRC;		//CRC循环冗余校验码
	int acceptfd;	//客户端网络连接描述符
}REBUILD_HEAD;

//----------------------------------------------------------协议体------------------------------------
//协议体1
//业务： 客户端登录 → 服务器 
typedef struct login
{
	char id[20];			//用户账号（id）
	char password[50];		//用户密码（MD5加密后的）
}LOGIN;

//协议体2
//业务： 客户端注册 → 服务器 
typedef struct registerr
{
	char id[20];			//用户账号（id）
	char name[20];			//用户昵称
	char password[50];		//用户密码（MD5加密后的）
}REGISTER;

//协议体3
//业务： 视频信息上传 → 服务器
typedef struct videoInfo
{
	char videoPath[50];				//视频存储路径
	char videoCoverPath[50];		//视频封面存储路径
	char videoTime[30];				//视频存储时间 （客户端无需上传，服务器写入数据时记当前时间）
	int curFrame;					//视频播放当前帧数
	int totalFrame;					//视频总帧数
}VIDEO_INFO;

//协议体4
//业务：图片 与 图片信息 上传→服务器
typedef struct photoInfo
{
	char photoPath[50];				//图片存储路径
	char traitPhotoTime[30];		//特征图片存储时间（客户端无需上传，服务器写入数据时记当前时间）
	char p_fileName[60];			//文件名称
	char p_filePageContext[1000];	//文件包内容
	int p_filePageLen;              //文件包长度
	int p_filePageIndex;			//当前文件包下标
	int p_fileSize;					//文件总大小
	int p_filePageNum;				//碎片文件总个数
}PHOTO_INFO;

//协议体9
//业务：特征图片信息请求包
typedef struct  photoAsk
{
	int photo_index;
}PHOTO_ASK;

//协议体10
//业务：特征视频信息请求包
typedef struct  videoAsk
{
	int video_index;
}VIDEO_ASK;

//----------------------------------------------------------服务器反馈包→客户端反馈包

//协议体5
//服务器对登录的反馈→客户端
typedef struct loginBack
{
	int result;					//返回结果: 1.成功  -1.失败  0.空（例：服务器没有找到 用户信息/视频信息/图片信息时 给出为空的反馈）
	char name[20];				//用户昵称
	int page;					//需要的视频信息列表的页数 （按每页6个数据计算） 默认为 第一页的下标
	VIDEO_INFO videoArr[6];		//视频信息结构体数组 默认6个
}LOGINBACK;

//协议体6
//服务器对请求结果的反馈→客户端
typedef struct backPack
{
	int result;					//返回结果: 1.成功  -1.失败  0.空（例：服务器没有找到 用户信息/视频信息/图片信息时 给出为空的反馈）
}BACK_PACK;

//协议体7
//服务器对视频信息的反馈→客户端
typedef struct getVideosInfo
{
	int allVideo;               //返回视频总个数
	int page;					//需要的视频信息列表的页数 （按每页6个数据计算）
	VIDEO_INFO videoArr[6];		//视频信息结构体数组 默认6个
}GET_VIDEOS_INFO;

//协议体8
//服务器对图片信息的反馈→客户端
//图片信息结构体
typedef struct photo
{
	char photoPath[50];			//图片存储路径
	char traitPhotoTime[30];	//特征图片存储时间
}PHOTO;

//图片信息返回包
typedef struct getPhotosInfo
{
	int allPhoto;               //返回图片总个数
	int page;					//需要的图片信息列表的页数 （按每页6个数据计算）//返回总页数，用于边界判断
	PHOTO photoArr[6];			//图片信息结构体数组 默认6个
}GET_PHOTOS_INFO;

