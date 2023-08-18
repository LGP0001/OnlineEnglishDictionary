#ifndef CONCURRENT_H
#define CONCURRENT_H

#include <pthread.h>

// 数据结构、常量、宏定义
#define MAX_THREADS 10
// 为了简化，我假设服务器端的最大客户端数量为100
#define MAX_CLIENTS 100

// 存储客户端套接字描述符的全局数组
extern int client_sockets[MAX_CLIENTS];

// 全局锁
extern pthread_mutex_t global_lock;
// 日志锁
extern pthread_mutex_t log_lock;
// 初始化并发处理模块
void initialize_concurrent_module(int num_threads);
// 客户端处理线程函数
void *client_handler_thread(void *arg);
// 添加新的客户端到客户端套接字数组
int add_client(int client_socket);
// 删除客户端套接字数组中的一个客户端
void remove_client(int client_socket);

#endif // CONCURRENT_H