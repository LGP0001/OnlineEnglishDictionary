#include "logs.h"
#include <stdio.h>
#include <time.h>

static FILE *logFile = NULL;
// 初始化日志模块
void Init_Logger(const char *logFilePath)
{
    if(logFile)
        fclose(logFile);

    logFile = fopen(logFilePath, "a");
}
// 记录错误
void LogError(const char *message)
{
    if(logFile)
        fprintf(logFile, "[ERROR] [%s] %s\n", GetCurrentTimestamp(), message);

    fprintf(stderr, "[ERROR] %s\n", message);
}

// 记录信息
void LogInfo(const char *message) 
{
    if (logFile) {
        fprintf(logFile, "[INFO] [%s] %s\n", GetCurrentTimestamp(), message);
    }
    fprintf(stdout, "[INFO] %s\n", message);
}

// 获取当前时间戳
static const char* GetCurrentTimestamp() 
{
    static char buffer[20];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);

    if (tmp == NULL) {
        return "";
    }

    if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp) == 0) {
        return "";
    }
    return buffer;
}
