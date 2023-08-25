#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include <stdbool.h>


// 自定义协议类型
typedef enum 
{
	    // 用户请求
    REGISTER = 1,            // 注册请求
    LOGIN,               // 登录请求
    QUERY_WORD,          // 查询单词请求
    VIEW_USER_HISTORY,   // 查看个人查询历史请求

    // 管理员请求
    VIEW_ALL_HISTORY,    // 查看所有用户查询历史请求
    // 新增的协议类型
    ACK,                  // 确认消息
    EXIT                 // 退出信息
}Type;

// 请求结构体，可能会包含如用户名、密码、查询的单词等信息
typedef struct 
{
    Type type;
    char username[50];
    char password[50];
    char query_word[100];
    bool is_admin;  // 标记是否是管理员
} Request;

// 响应结构体，可能会包含如单词的定义、查询历史、登录/注册状态等信息
typedef struct 
{
    Type type;             
    char word[100];        // 单词本身
    char definition[500];  // 单词的定义或释义
    char history[1000];    // 查询历史
    bool login_status;     // 登录成功或失败
    bool register_status;  // 注册成功或失败
    bool is_admin;  // 标记是否是管理员
} Response; 

typedef struct
{
    int type;
    char name[32];
    char data[256];
} MSG;
// 查看历史缓冲区
typedef struct 
{
    char *buffer; // 存放历史记录的缓冲区
    int buffer_length; // 缓冲区的大小
    int current_length; // 当前已使用的缓冲区大小
} HistoryBufferContext;
#endif // COMMON_STRUCTS_H
