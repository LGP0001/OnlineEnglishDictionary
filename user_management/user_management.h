#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

// 可以根据需要增加其他头文件引用
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char username[50];
    char password[50];
    char role[10]; // "admin" 或 "user"
} User;

// 初始化用户表
void setup_user_database();
// 用户注册功能
bool register_user(User *user);

// 用户登录功能
bool login_user(User *user);

#endif // USER_MANAGEMENT_H