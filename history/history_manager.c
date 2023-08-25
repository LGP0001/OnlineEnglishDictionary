#include "../history/history_manager.h"

// 创建查询历史表
void CreateHistoryTable(sqlite3* db)
{
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS query_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "query_word TEXT NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
        Execute_SQL(db, create_table_sql, NULL, NULL);
}
// 添加查询记录到历史表
int AddEntryToHistory(sqlite3* db, const char* username, const char* word)
{
    char sql[256];
    sprintf(sql, "INSERT INTO query_history (username, query_word) VALUES ('%s', '%s');", username, word);
    return Execute_SQL(db, sql, NULL, NULL);
}

int HistoryCallback(void* data, int argc, char** argv, char** azColName) 
{
    HistoryBufferContext* context = (HistoryBufferContext*)data;

    for (int i = 0; i < argc; i++) 
    {
        int needed_length = snprintf(NULL, 0, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (context->current_length + needed_length < context->buffer_length) 
        {
            context->current_length += snprintf(context->buffer + context->current_length, context->buffer_length - context->current_length, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        else 
        {
            LogError("缓冲区太小了");             // 缓冲区太小了，我们不能添加更多的内容了
            return 1; // 表示错误
        }
    }

    return 0;
}
//  获取所有的查询历史
void DisplayHistory(sqlite3* db, void* context) 
{
    const char* select_sql = "SELECT query_word, timestamp FROM query_history ORDER BY timestamp DESC;";
    Execute_SQL(db, select_sql, HistoryCallback, context);
}
// 获取个人的查询历史
void DisplayHistoryForUser(sqlite3* db, const char* username, HistoryBufferContext* context)
{
    const char* select_sql = "SELECT query_word, timestamp FROM query_history WHERE username=? ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL) != SQLITE_OK) {
        // 处理错误，例如通过日志记录
        return;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* query_word = sqlite3_column_text(stmt, 0);
        const unsigned char* timestamp = sqlite3_column_text(stmt, 1);
        
        // 将查询历史添加到缓冲区
        int written = snprintf(context->buffer + context->current_length, context->buffer_length - context->current_length, "%s - %s\n", query_word, timestamp);
        
        if (written > 0 && written < context->buffer_length - context->current_length) {
            context->current_length += written;
        }
        // 如果缓冲区空间不足，您可能需要进行其他处理，如截断数据或终止操作
    }
    
    sqlite3_finalize(stmt);
}

int HistoryCallbackForUser(void* context, int argc, char** argv, char** azColName) 
{
    HistoryBufferContext* bufferContext = (HistoryBufferContext*) context;

    // 将查询结果追加到缓冲区
    int written = snprintf(bufferContext->buffer + bufferContext->current_length, bufferContext->buffer_length - bufferContext->current_length, "%s - %s\n", argv[0], argv[1]);
    
    if (written > 0 && written < bufferContext->buffer_length - bufferContext->current_length) {
        bufferContext->current_length += written;
    }

    return 0; // 继续执行查询
}
// 清除查询历史
void ClearHistory(sqlite3* db)
{
    const char* clear_sql = "DELETE FROM query_history;";
    Execute_SQL(db, clear_sql, NULL, NULL);
}