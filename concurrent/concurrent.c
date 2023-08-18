#include "../concurrent/concurrent.h"
#include "../logs/logger.c"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER; // 初始化锁
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;// 初始化日志锁
static FILE *logFile = NULL;


// 初始化并发处理模块
void initialize_concurrent_module(int num_threads)
{
    pthread_t threads[MAX_THREADS];

    if (num_threads > MAX_THREADS)
    {
        pthread_mutex_lock(&log_lock);
        LogError("超过最大线程数");
        pthread_mutex_unlock(&log_lock);
        return;// 如果超出线程限制，直接返回
    }
    // 创建线程
    for (int i = 0; i < num_threads; i++)
    {
        int thread_num= i + 1;// 分配线程号
        if((pthread_create(&threads[i], NULL, client_handler_thread, (void*)(size_t)thread_num)) != 0)
        {
            pthread_mutex_lock(&log_lock);
            LogError("线程创建失败");
            pthread_mutex_unlock(&log_lock);
            return;// 如果线程创建失败，终止进程也是一个选择
        }
    }
    // 等待线程完成
    for (int i = 0; i < num_threads; i++) 
    {
        if(pthread_join(threads[i], NULL) != 0)
        {
            pthread_mutex_lock(&log_lock);
            LogError("线程连接失败");
            pthread_mutex_unlock(&log_lock);
            return; // 如果线程连接失败，您可能需要考虑如何处理
        }
    }
    LogInfo("并发处理模块初始化完成"); 
}
// 客户端处理线程函数
void *client_handler_thread(void *arg)
{
    int client_socket = *(int*)arg;
    char buffer[BUFSIZ];
    int read_size;

    while((read_size = recv(client_socket, buffer, sizeof(buffer),0)) >0)
    {
        send(client_socket, buffer, read_size, 0);
    }
    if(read_size == 0)
    {
        pthread_mutex_lock(&log_lock);
        LogInfo("客户端断开连接");
        pthread_mutex_unlock(&log_lock);
        close(client_socket);
        remove_client(client_socket);
    }
    else if (read_size == -1)
    {
        pthread_mutex_lock(&log_lock);
        LogError("接收错误");
        pthread_mutex_unlock(&log_lock);
    }
    
    return NULL;
}




