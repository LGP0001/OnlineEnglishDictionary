// database.h

#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

// 函数声明
// 初始化数据库连接
sqlite3* Init_Database(const char* path);
// 执行SQL命令
int Execute_SQL(sqlite3* db, const char *sql, int (*callback)(void*, int, char**, char**), void* data);
// 关闭数据库
void Close_Database(sqlite3 *db);

#endif // DATABASE_H