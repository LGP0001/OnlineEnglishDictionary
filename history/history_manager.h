#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "../logs/logs.h"
#include "../utils/database.h"
#include "../utils/common_structs.h"
#include <stdio.h>

// 创建查询历史表
void CreateHistoryTable(sqlite3* db);
// 添加查询记录到历史表
int AddEntryToHistory(sqlite3* db, const char* username, const char* word);
//  获取所有的查询历史
void DisplayHistory(sqlite3* db, void* context);
// 获取个人的查询历史
void DisplayHistoryForUser(sqlite3* db, const char* username, HistoryBufferContext* context);
// 清除查询历史
void ClearHistory(sqlite3* db);

#endif