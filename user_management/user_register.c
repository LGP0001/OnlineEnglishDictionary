#include "../utils/database.h"
#include "../user_management/user_management.h"
#include "../logs/logs.h"

sqlite3* db;
void setup_user_database()
{
    db = Init_Database("../dictionary_query/users.db");
    // 创建users表格；
    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, role TEXT DEFAULT 'user');";    
    Execute_SQL(db, create_table_sql, NULL, NULL);
    // 插入管理员账号
    const char* insert_admin_sql = "INSERT OR IGNORE INTO users (username, password, role) VALUES ('root', '1', 'admin');";
    Execute_SQL(db, insert_admin_sql, NULL, NULL);
}

bool register_user(User *user)
{
    char sql[256];
    sprintf(sql, "INSERT INTO users (username, password, role) VALUES ('%s', '%s','user');", user->username, user->password);

    if (Execute_SQL(db, sql, NULL, NULL) != SQLITE_OK)
    {
        LogError("用户注册失败。可能的原因：用户名已存在。");
        return false;
    }
    LogInfo("用户注册成功");
    return true;
}