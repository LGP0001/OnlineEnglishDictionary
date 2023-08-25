#ifndef CONCURRENT_H
#define CONCURRENT_H

#include <pthread.h>
#include <stdbool.h>

// 数据结构、常量、宏定义
#define MAX_THREADS 10
// 为了简化，我假设服务器端的最大客户端数量为100
#define MAX_CLIENTS 100

// 客户端队列
extern int client_queue[MAX_CLIENTS];
extern int queue_front, queue_rear; // 队列的前端和尾端
extern pthread_mutex_t queue_lock;  // 队列互斥锁

/*线程操作*/
// 全局锁
extern pthread_mutex_t global_lock;
// 日志锁
extern pthread_mutex_t log_lock;
// 初始化并发处理模块
void initialize_concurrent_module(int num_threads);
// 客户端处理线程函数
void *client_handler_thread(void *arg);
// 添加客户端队列元素
void enqueue_client(int client_socket);
// 取出客户端队列元素
int dequeue_client(void);
// 移除一个已经处理完的客户端队列元素
void remove_client(int client_socket);
// 关闭所有线程
void shutdown_server(int num_threads);

/*请求操作*/
// 处理注册请求
// void handle_register_request(int client_socket, Request *request);
// 处理登录请求
// void handle_login_request(int client_socket, Request* request);
// 处理管理员请求
// void handle_admin_actions(int client_socket);
// 处理普通用户请求
// void handle_user_actions(int client_socket);




#endif // CONCURRENT_H