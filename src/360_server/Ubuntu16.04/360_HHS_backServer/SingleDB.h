#pragma once

#include <iostream>
#include <sqlite3.h>

using namespace std;

//数据库单例
class SingleDB
{
public:
    static SingleDB* getInstance();                                 //外部接口--获取该类唯一实例
    int execSql(char* sqlCode, char**& result, int& row, int& col); //执行sql语句   返回1-sql语句执行成功   返回0-sql语句执行失败
private:
    SingleDB();                        //私有构造函数--保证该类只有一个实例
    ~SingleDB();

    static SingleDB* dataBase;         //该类唯一实例
    sqlite3* sqlDB;                    //数据库指针
};

