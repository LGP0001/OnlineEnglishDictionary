#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include "../utils/utils.h"
#include "../logs/logs.h"
#include "../logs/logger.c"
#include "../utils/networking.c"
#include "../utils/database.c"

// 测试 Init_Address
void test_Init_Address() 
{
    int client_socket;
    int server_socket;

    // 测试1: 服务器模式
    server_socket = Init_Address("127.0.0.1", "8080", true);
    assert(server_socket != ERROR);
    LogInfo("测试1: 服务器模式成功!");

    // 测试2: 客户端模式
    client_socket = Init_Address("127.0.0.1", "8080", false);
    assert(client_socket != ERROR);
    LogInfo("测试2: 客户端模式成功!");

    // 测试3: 无效的IP
    int invalid_socket = Init_Address("256.256.256.256", "8080", true);
    assert(invalid_socket == ERROR);
    LogInfo("测试3: 无效IP测试成功!");

    // 测试4: 已被使用的端口 (这里只是一个例子，确保端口80已被使用)
    server_socket = Init_Address("127.0.0.1", "80", true);
    assert(server_socket == ERROR);
    LogInfo("测试4: 已被使用的端口测试成功!");

    // 测试5: 非法端口
    invalid_socket = Init_Address("127.0.0.1", "70000", true);
    assert(invalid_socket == ERROR);
    LogInfo("测试5: 非法端口测试成功!");

    // 测试6: 非服务器模式下尝试绑定
    invalid_socket = Init_Address("127.0.0.1", "8082", false);
    assert(invalid_socket == ERROR);
    LogInfo("测试6: 非服务器模式绑定测试成功!");

    printf("All tests for Init_Address passed!\n");
}
// Mock回调函数，用于SQLite
int mock_callback(void* not_used, int argc, char** argv, char** azColName)
{
    return 0;
}
// 测试数据库的打开与关闭功能
void test_Init_and_Close_Database() 
{
    const char* test_db_path = "test.db";  //测试用的数据库路径

    // 使用Init_Database函数打开数据库
    sqlite3* db = Init_Database(test_db_path);
    // 断言确保数据库已成功打开
    assert(db != NULL && "数据库初始化失败.");

    // 使用Close_Database函数关闭数据库
    Close_Database(db);
    LogInfo("测试1: 数据库打开与关闭成功!");
}
// 测试有效的SQL命令
void test_Valid_SQL_Command() {
    const char* test_db_path = "test.db";  //测试用的数据库路径
    const char* valid_sql = "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);";  // 有效的SQL语句

    // 使用Init_Database函数打开数据库
    sqlite3* db = Init_Database(test_db_path);
    assert(db != NULL && "用于有效SQL命令测试的数据库初始化失败.");

    // 使用Execute_SQL函数执行SQL命令
    int rc = Execute_SQL(db, valid_sql, mock_callback, NULL);
    // 断言确保SQL命令已成功执行
    assert(rc == SQLITE_OK && "有效SQL命令执行失败.");

    // 关闭数据库
    Close_Database(db);
    LogInfo("测试2: 有效的SQL命令执行成功!");
}
// 测试无效的SQL命令
void test_Invalid_SQL_Command() 
{
    const char* test_db_path = "test.db";  //测试用的数据库路径
    const char* invalid_sql = "CREAT TABLE invalid_test (id INT, name TXT);";  // 故意拼写错误的SQL语句

    // 使用Init_Database函数打开数据库
    sqlite3* db = Init_Database(test_db_path);
    assert(db != NULL && "用于无效SQL命令测试的数据库初始化失败.");

    // 使用Execute_SQL函数执行SQL命令
    int rc = Execute_SQL(db, invalid_sql, mock_callback, NULL);
    // 断言确保SQL命令执行失败
    assert(rc != SQLITE_OK && "无效SQL命令应失败但并未失败.");

    // 关闭数据库
    Close_Database(db);
    LogInfo("测试3: 无效的SQL命令测试成功!");
}

int main() 
{
//  test_Init_Address();// 
    test_Init_and_Close_Database();  
    test_Valid_SQL_Command();  
    test_Invalid_SQL_Command();  

    LogInfo("所有函数的测试均已通过!");
    return 0;
}