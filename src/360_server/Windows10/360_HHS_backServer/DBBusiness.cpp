#include "DBBusiness.h"

DBBusiness* DBBusiness::dbBusiness = NULL;
DBBusiness* DBBusiness::getInstance()
{
    if (NULL == DBBusiness::dbBusiness)
    {
        DBBusiness::dbBusiness = new DBBusiness();
    }
    return DBBusiness::dbBusiness;
}

int DBBusiness::searchUserId(char** &qres, int& row, int& col, const char userId[],const char userPwd[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：获取用户ID，判断登录是否成功，返回用户昵称" << endl;

    //拼接sql语句
    char sqlCode[] = "select userId from tbl_userInfo where userId='%s' and userPwd='%s';";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId, userPwd);

    //必要参数
    char** judgeQres = nullptr;
    int judgeRow = 0, judgeCol = 0;

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, judgeQres, judgeRow, judgeCol);
    cout << "getUserNameAndPwd res = " << res << endl;
    if (1 == res)
    {
        //获取用户昵称
        //拼接sql语句
        char userNameSqlCode[] = "select userName from tbl_userInfo where userId='%s' and userPwd='%s';";
        char newUserNameSqlCode[strlen(userNameSqlCode)];
        sprintf(newUserNameSqlCode, userNameSqlCode, userId, userPwd);

        //执行sql语句
        int rres = SingleDB::getInstance()->execSql(newUserNameSqlCode, qres, row, col);
        if (rres == 1)
        {
            return judgeRow; //1：客户端登录成功；0：客户端登录失败
        }
        else
        {
            return -1;  //sql语句执行失败
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::addNewUser(const char userId[], const char userName[], const char userPwd[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端注册业务" << endl;

    //拼接sql语句
    char sqlCode[] = "select userId from tbl_userInfo where userId='%s';";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId);

    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;

    //执行sql语句
    int judgeRes = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "addNewUser judgeRes = " << judgeRes << endl;
    if (1 == judgeRes)
    {
        if (0 == row)   //1：用户Id已存在，注册失败；0：用户Id不存在，进行注册业务
        {
            //注册业务
            //拼接sql语句
            char registSqlCode[] = "insert into tbl_userInfo(userId,userName,userPwd) values('%s','%s','%s');";
            char newRegistSqlCode[strlen(registSqlCode)];
            sprintf(newRegistSqlCode, registSqlCode, userId, userName, userPwd);

            //必要参数
            char** qres = nullptr;
            int row = 0, col = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->execSql(newRegistSqlCode, qres, row, col);
            cout << "addNewUser res = " << res << endl;
            if (1 == res)
            {
                return 1;   //注册成功
            }
            else
            {
                return -1;  //sql执行语句失败
            }
        }
        else
        {
            return 0;   //注册失败
        }
    }
    else
    {
        return -1;      //sql语句执行失败
    }

}

int DBBusiness::addNewPhoto(const char userId[], const char imgPath[], const char imgSaveTime[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端图片上传业务" << endl;

    //拼接sql语句
    char sqlCode[] = "insert into tbl_imageInfo(imgPath,imgSaveTime,userId) values('%s','%s','%s');";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, imgPath, imgSaveTime, userId);

    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "addNewPhoto res = " << res << endl;
    if (1 == res)
    {
        return 1;   //图片上传成功
    }
    else
    {
        return -1;  //sql执行语句失败
    }

    cout << "------------------------------------------------" << endl;
}

int DBBusiness::searchPhoto(char**& qres, int& row, int& col, const char userId[], const int pageIndex)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端图片信息检索业务" << endl;

    //拼接sql语句
    char sqlCode[] = "select * from tbl_imageInfo where userId='%s' limit 6 offset 6*%d;";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId, pageIndex);

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "searchPhoto res = " << res << endl;
    if (1 == res)
    {
        //返回用户当前页图片数据个数
        //拼接sql语句
        char sqlCode2[] = "select * from tbl_imageInfo where userId='%s'";
        char newSqlCode2[strlen(sqlCode2)];
        sprintf(newSqlCode2, sqlCode2, userId);

        //必要参数
        char** qres2 = nullptr;
        int row2 = 0, col2 = 0;

        //执行sql语句
        int rres = SingleDB::getInstance()->execSql(newSqlCode2, qres2, row2, col2);
        cout << "searchPhoto res = " << res << endl;
        if (1 == rres)
        {
            return row2; //0：未检索到图片信息；其他：检索到图片信息
        }
        else
        {
            return -1;  //sql语句执行失败
        } 
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::addNewVideo(const char userId[], const char videoPath[], const char videoSaveTime[], const int videoCurFrame, const int videoTotalFrame, const char videoCoverPath[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：新增视频信息业务" << endl;

    //拼接sql语句
    char sqlCode[] = "select * from tbl_videoInfo where userId='%s' and videoPath='%s'";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId, videoPath);

    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "addNewVideo res = " << res << endl;
    if (1 == res)
    {
        //视频信息不存在，做插入
        if (0 == row)       
        {
            //拼接sql语句
            char insertSqlCode[] = "insert into tbl_videoInfo(videoPath,videoSaveTime,videoCurFrame,videoTotalFrame,videoCoverPath,userId) values('%s','%s',%d,%d,'%s','%s');";
            char insertNewSqlCode[strlen(insertSqlCode) * 2];
            sprintf(insertNewSqlCode, insertSqlCode, videoPath, videoSaveTime, videoCurFrame, videoTotalFrame, videoCoverPath, userId);
            
            //必要参数
            char** insertQres = nullptr;
            int insertRow = 0, insertCol = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->execSql(insertNewSqlCode, insertQres, insertRow, insertCol);
            cout << "addNewVideo res = " << res << endl;
            if (1 == res)
            {
                return 1;   //视频信息添加成功
            }
            else
            {
                return -1;  //sql语句执行失败
            }
        }
        //视频信息存在，做更新
        else                
        {
            //拼接sql语句
            char updateSqlCode[] = "update tbl_videoInfo set videoCurFrame=%d,videoTotalFrame=%d where userId='%s' and videoPath='%s';";
            char updateNewSqlCode[strlen(updateSqlCode) * 2];
            sprintf(updateNewSqlCode, updateSqlCode, videoCurFrame, videoTotalFrame, userId, videoPath);

            //必要参数
            char** updateQres = nullptr;
            int updateRow = 0, updateCol = 0;

            //执行sql语句
            int res = SingleDB::getInstance()->execSql(updateNewSqlCode, updateQres, updateRow, updateCol);
            cout << "addNewVideo res = " << res << endl;
            if (1 == res)
            {
                return 1;   //视频信息添加成功
            }
            else
            {
                return -1;  //sql语句执行失败
            }
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::searchVideo(char**& qres, int& row, int& col, const char userId[], const int pageIndex)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：客户端视频信息检索业务" << endl;

    //拼接sql语句
    char sqlCode[] = "select * from tbl_videoInfo where userId='%s' limit 6 offset 6*%d;";
    char newSqlCode[strlen(sqlCode)];
    sprintf(newSqlCode, sqlCode, userId, pageIndex);

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "searchVideo res = " << res << endl;
    if (1 == res)
    {
        //返回用户所有视频数据个数
        //拼接sql语句
        char sqlCode2[] = "select * from tbl_videoInfo where userId='%s'";
        char newSqlCode2[strlen(sqlCode2)];
        sprintf(newSqlCode2, sqlCode2, userId);

        //必要参数
        char** qres2 = nullptr;
        int row2 = 0, col2 = 0;

        //执行sql语句
        int rres = SingleDB::getInstance()->execSql(newSqlCode2, qres2, row2, col2);
        cout << "searchPhoto res = " << res << endl;
        if (1 == rres)
        {
            return row2; //0：未检索到视频信息；其他：检索到视频信息
        }
        else
        {
            return -1;  //sql语句执行失败
        }
    }
    else
    {
        return -1;  //sql语句执行失败
    }
}

int DBBusiness::addNewLog(const char userId[], const char operationTime[], const char operationRecord[], const char operationPath[])
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：新增日志业务" << endl;

    //拼接sql语句
    char sqlCode[] = "insert into tbl_logInfo(operationTime,operationRecord,operationPath,userId) values('%s','%s','%s','%s');";
    char newSqlCode[strlen(sqlCode)*2];
    sprintf(newSqlCode, sqlCode, operationTime, operationRecord, operationPath, userId);

    //必要参数
    char** qres = nullptr;
    int row = 0, col = 0;

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(newSqlCode, qres, row, col);
    cout << "addNewLog res = " << res << endl;
    if (1 == res)
    {
        return 1;   //日志写入成功
    }
    else
    {
        return -1;  //sql语句执行失败
    }

    return 0;
}

int DBBusiness::searchLog(char**& qres, int& row, int& col)
{
    cout << "------------------------------------------------" << endl;
    cout << "数据库业务：日志检索业务导出" << endl;

    //拼接sql语句
    char sqlCode[] = "select * from tbl_logInfo;";

    //执行sql语句
    int res = SingleDB::getInstance()->execSql(sqlCode, qres, row, col);
    cout << "searchLog res = " << res << endl;
    if (1 == res)
    {
        return row;  //0：未检索到对应数据；其他：检索到对应日志数据
    }
    else
    {
        return -1;   //sql语句执行失败
    }

    return 0;
}

DBBusiness::DBBusiness()
{
}