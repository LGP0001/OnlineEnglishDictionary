#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include <stdbool.h>

// 请求结构体，可能会包含如用户名、密码、查询的单词等信息
typedef struct 
{
    char username[50];
    char password[50];
    char query_word[100];
    bool is_admin;  // 标记是否是管理员
} Request;

// 响应结构体，可能会包含如单词的定义、查询历史、登录/注册状态等信息
typedef struct 
{
    char word[100];        // 单词本身
    char definition[500];  // 单词的定义或释义
    char history[1000];    // 查询历史
    bool login_status;     // 登录成功或失败
    bool register_status;  // 注册成功或失败
} Response;

typedef struct
{
    int type;
    char name[32];
    char data[256];
} MSG;
#endif // COMMON_STRUCTS_H
