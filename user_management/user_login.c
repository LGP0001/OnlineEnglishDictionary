#include "../utils/database.h"
#include "../user_management/user_management.h"
#include "../logs/logs.h"

bool is_authenticated = false;
sqlite3 *db;

int login_callback(void* data, int argc, char** argv, char** azColName)
{
    User* user = (User*) data;
    // 如果查询返回结果，说明用关乎存在并密码正确
    if(argc > 0)
    {
        is_authenticated = true; // 在这里获取用户角色，并保存在全局变量或者user结构体中
        strncpy(user->role, argv[2], sizeof(user->role));
    }
    return 0;
}

bool login_user(User *user) 
{
    char sql[256];
    is_authenticated = false;  // 重置认证标志
    
    sprintf(sql, "SELECT * FROM users WHERE username='%s' AND password='%s';", user->username, user->password);
    
    Execute_SQL(db, sql, login_callback, user); // 我们传递user作为参数给回调函数

    if (is_authenticated) 
    {
        if (strcmp(user->role, "admin") == 0)
        {
            LogInfo("管理员登陆成功");
        }
        else
        {
            LogInfo("用户登陆成功，欢迎回来");
        }
        
        return true;
    }
    else 
    {
        LogError("用户名或密码错误");
        return false;
    }
}