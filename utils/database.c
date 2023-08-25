#include "../utils/utils.h"
#include "../utils/database.h"
#include "../logs/logs.h"

// 初始化数据库连接
sqlite3* Init_Database(const char* path)
{
    sqlite3* db;
    int rc = sqlite3_open(path, &db);// 打开数据库

    if(rc)
    {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "无法打开数据库. 错误原因: %s", sqlite3_errmsg(db));
        LogError(error_msg);
        return NULL;
    }
    else
    {
        LogInfo("成功打开数据库");
        return db;
    }  
}
// 执行SQL命令
int Execute_SQL(sqlite3* db, const char *sql, int (*callback)(void*, int, char**, char**), void* data)
{
    char *errmsg = 0;
    int rc = sqlite3_exec(db, sql, callback, data, &errmsg);

    if(rc != SQLITE_OK)
    {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "SQL命令错误, 错误原因: %s", errmsg);
        LogError(error_msg);
        sqlite3_free(errmsg);
    }
    else
    {
        LogInfo("SQL命令执行成功");
    }
    return rc;
}

void Close_Database(sqlite3 *db) 
{
    if(db) {
        sqlite3_close(db);
        LogInfo("数据库关闭成功");
    }
}
    