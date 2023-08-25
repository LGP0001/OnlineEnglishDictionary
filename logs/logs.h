#ifndef LOGS_H
#define LOGS_H
/*在头文件中定义日志路径，建议定义一个函数或宏来动态生成完整的日志路径，而不是直接定义一个字符串常量*/
#define SERVER_LOG_FILE_PATH "/home/lgp/桌面/work/project/OnlineEnglishDictionary/logs/server_error.log"
#define CLIENT_LOG_FILE_PATH "/home/lgp/桌面/work/project/OnlineEnglishDictionary/logs/client_error.log"
// 初始化日志模块
void Init_Logger(const char *logFilePath);

// 记录错误
void LogError(const char *message);

// 记录信息
void LogInfo(const char *message);

#endif // LOGS_H

