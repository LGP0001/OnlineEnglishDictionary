#include "../history/history_manager.h"
#include "../history/history_manager.c"
#include <assert.h>

int main(int argc, const char *argv[]) 
{
    sqlite3* db = Init_Database("test_database.db");
    assert(db != NULL);
    LogInfo("测试1: 数据库初始化成功\n");

    CreateHistoryTable(db);
    LogInfo("测试2: 查找历史表创建成功\n");

    int result = AddEntryToHistory(db, "test_word");
    assert(result == SQLITE_OK);
    LogInfo("测试3: 记录添加成功\n");

    printf("获取查询历史：\n");
    DisplayHistory(db);
    LogInfo("测试4: 查询历史成功\n");

    ClearHistory(db);
    LogInfo("测试5: 历史记录清除成功\n");

    Close_Database(db);
    LogInfo("测试6: 数据库关闭成功\n");

    LogInfo("所有测试均已通过！");
}
