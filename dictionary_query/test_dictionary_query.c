#include <assert.h>
#include "../utils/database.h"
#include "../logs/logs.h"
#include "../dictionary_query/user_register.c"
#include "../dictionary_query/user_login.c"


int main(int argc, const char *argv[]) 
{
    User test_user;

    // 设置测试数据库
    setup_user_database();

    // 测试用户注册
    strcpy(test_user.username, "testUser");
    strcpy(test_user.password, "testPassword");
    bool register_result = register_user(&test_user);
    assert(register_result == true);
    LogInfo("测试1: 用户注册成功");

    // 测试用户登录
    bool login_result = login_user(&test_user);
    assert(login_result == true);
    LogInfo("测试2: 用户登陆成功");

    // 测试错误密码
    strcpy(test_user.password, "wrongPassword");
    login_result = login_user(&test_user);
    assert(login_result == false);
    LogInfo("错误密码测试成功");

    // 测试错误用户名
    strcpy(test_user.username, "wrongtestUser");
    strcpy(test_user.password, "testPassword");
    login_result = login_user(&test_user);
    assert(login_result == false);
    LogInfo("错误用户名通过");

    LogInfo("所有测试均已通过！");
}