#include "SingleDB.h"

//静态成员定义
SingleDB* SingleDB::dataBase = NULL;

SingleDB* SingleDB::getInstance()
{
    if (nullptr == SingleDB::dataBase)
    {
        SingleDB::dataBase = new SingleDB();
    }
    return SingleDB::dataBase;
}

int SingleDB::execSql(char* sqlCode, char**& result, int& row, int& col)
{
    char* errmsg = nullptr;

    int res = sqlite3_get_table(sqlDB, sqlCode, &result, &row, &col, &errmsg);
    if (res == SQLITE_OK)
    {
        return 1;       //返回1-sql语句执行成功
    }
    else
    {
        cout << sqlite3_errmsg(sqlDB) << endl;
        cout << sqlite3_errcode(sqlDB) << endl;
        
        return 0;       //返回0-sql语句执行失败
    }
}

SingleDB::SingleDB()
{
    int res = sqlite3_open("360.db", &sqlDB);
    if (res == SQLITE_OK)
    {
        cout << "open dataBase success" << endl;
    }
    else
    {
        cout << sqlite3_errmsg(sqlDB) << endl;
        cout << sqlite3_errcode(sqlDB) << endl;
    }
}

SingleDB::~SingleDB()
{
    sqlite3_close(sqlDB);
}
