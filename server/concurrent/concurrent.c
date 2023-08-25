// #include "../../utils/common_structs.h"
#include "../../utils/utils.h"
#include "../../logs/logs.h"
#include "../../history/history_manager.h"
#include "../../dictionary_query/dictionary_query.h"
#include "../../user_management/user_management.h"
#include "../concurrent/concurrent.h"
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER; // 初始化锁
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;// 初始化日志锁
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
// static FILE *logFile = NULL;
volatile bool should_exit = false;
extern sqlite3 *db;
pthread_t threads[MAX_THREADS];

int client_queue[MAX_CLIENTS] = {0};
int queue_front = 0, queue_rear = -1, queue_count = 0;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

// 初始化并发处理模块
void initialize_concurrent_module(int num_threads)
{


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

    LogInfo("并发处理模块初始化完成"); 
}
// 客户端处理线程函数
void *client_handler_thread(void *arg)
{
    // sqlite3 *db = (sqlite3 *)arg;  // 传入数据库，如果你需要在这里访问它

    while (!should_exit)  // 让线程持续运行
    {
        int client_socket = dequeue_client();

        if (client_socket == -1)
        {
            // 队列为空，线程暂停一段时间，然后再次尝试
            LogInfo("队列空了");
            sleep(1);
            continue;
        }
        else
            LogInfo("队列中有嵌套字");

        Request request;
        int read_size = recv(client_socket, &request, sizeof(Request),0);

        while(read_size  > 0)
        {
            switch(request.type)
            {
                case REGISTER:
                    handle_register_request(client_socket, &request);
                    break;
                case LOGIN:
                    handle_login_request(client_socket, &request);
                    break;
                default:
                    LogError("未知的请求类型");
                    break;
            }
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
    }

    return NULL;
}
// 添加客户端队列元素
void enqueue_client(int client_socket) 
{
    pthread_mutex_lock(&queue_lock);

    if(queue_count == MAX_CLIENTS) 
    {
        // 队列满了
        LogError("客户端队列满了");
        // 可能需要关闭client_socket，避免资源泄露
        close(client_socket);
    } 
    else 
    {
        queue_rear = (queue_rear + 1) % MAX_CLIENTS;
        client_queue[queue_rear] = client_socket;
        queue_count++;
    }

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
}
// 取出客户端队列元素
int dequeue_client(void) 
{
    pthread_mutex_lock(&queue_lock);
    while(queue_count == 0 && !should_exit) 
    {
        pthread_cond_wait(&queue_cond, &queue_lock);
    }
    if (should_exit)
    {
        pthread_mutex_unlock(&queue_lock);
        return -1;
    }
    
    int client_socket = client_queue[queue_front];
    queue_front = (queue_front + 1) % MAX_CLIENTS;
    queue_count--;

    pthread_mutex_unlock(&queue_lock);
    return client_socket;
}
// 移除一个已经处理完的客户端队列元素
void remove_client(int client_socket) 
{
    pthread_mutex_lock(&queue_lock);
    
    int i;
    int found = -1;
    for(i = queue_front; i != queue_rear; i = (i + 1) % MAX_CLIENTS) 
    {
        if(client_queue[i] == client_socket) 
        {
            found = i;
            break;
        }
    }
    
    if(found != -1) 
    {
        // 移除 client_socket，并将其后面的元素都向前移动一个位置
        for(int j = found; j != queue_rear; j = (j + 1) % MAX_CLIENTS) 
        {
            client_queue[j] = client_queue[(j + 1) % MAX_CLIENTS];
        }
        
        // 更新 queue_rear 的位置
        if(queue_rear == 0) 
        {
            queue_rear = MAX_CLIENTS - 1;
        } 
        else 
        {
            queue_rear -= 1;
        }
        queue_count--;
    }
    
    pthread_mutex_unlock(&queue_lock);
}
// 关闭所有线程
void shutdown_server(int num_threads) 
{
    should_exit = true;  // 设置退出标志以通知工作线程结束其循环

    // 通知所有工作线程完成
    for (int i = 0; i < num_threads; i++) 
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            pthread_mutex_lock(&log_lock);
            LogError("线程连接失败");
            pthread_mutex_unlock(&log_lock);
            // 你可以选择如何处理这种情况，但一般情况下不应该直接返回
        }
    }
    // 这里还可以进行其他的清理工作，如关闭数据库连接、释放资源等
}
// 处理注册请求
#if 0
void handle_register_request(int client_socket, Request *request)
{
    Response res;
    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    User new_user;
    memset(&new_user, 0, sizeof(User)); // 初始化用户结构体
    
    strncpy(new_user.username, request->username, sizeof(new_user.username) - 1); // 从请求中复制用户名
    new_user.username[sizeof(new_user.username) - 1] = '\0';  // 确保字符串结束
    strncpy(new_user.password, request->password, sizeof(new_user.password) - 1); // 从请求中复制密码
    new_user.password[sizeof(new_user.password) - 1] = '\0';  // 确保字符串结束
    
    bool registration_status = register_user(&new_user);

    int max_retries = 3;
    int tries = 0;

    if (registration_status)
    {
        res.register_status = true;
    }
    else
    {
        res.register_status = false;
    }

    while(tries < max_retries)
    {
        if((send(client_socket, &res, sizeof(res), 0)) < 0)
        {
            LogError("向客户端发送注册状态信息失败");
            tries++;
        }
        else
        {
            break;
        }
    }
    
    if(tries == max_retries)
    {
        LogError("尝试发送给客户端多次失败，放弃发送");
        return;  // 直接从函数返回，不再执行下面的代码
    }
}
// 处理登录请求
void handle_login_request(int client_socket, Request* request)
{
    User user;
    memset(&user, 0, sizeof(User)); // 初始化用户结构体   

    strncpy(user.username, request->username, sizeof(user.username) - 1); // 从请求中复制用户名
    user.username[sizeof(user.username) - 1] = '\0';  // 确保字符串结束
    strncpy(user.password, request->password, sizeof(user.password) - 1); // 从请求中复制密码
    user.password[sizeof(user.password) - 1] = '\0';  // 确保字符串结束

    bool is_authenticated = login_user(&user);

    Response res;
    memset(&res, 0, sizeof(Response));
    res.login_status = is_authenticated;
    res.is_admin = (strcmp(user.role, "admin") == 0) ? true : false;

    int max_retries = 3;
    int tries = 0;

    // 向客户端发送登录的结果
    while(tries < max_retries)
    {
        if((send(client_socket, &res, sizeof(res), 0)) < 0)
        {
            LogError("向客户端发送登录状态信息失败");
            tries++;
        }
        else
        {
            LogInfo("向客户端发送登录状态信息成功");
            close(client_socket); // 关闭客户端套接字
            break;
        }
    }
    
    if(tries == max_retries)
    {
        LogError("尝试发送给客户端多次失败，放弃发送");
        return; // 直接从函数返回，不再执行下面的代码
    }

    if(is_authenticated)  // 管理员
        handle_admin_actions(client_socket);
    else 
        handle_user_actions(client_socket);
}
// 处理管理员请求
void handle_admin_actions(int client_socket)
{
    Request req; // 用于接收客户端请求
    Response res; // 用于发送给客户端的响应
    int max_retries = 3;
    int tries = 0;

    bool continue_running = true;

    while(continue_running)
    {
        memset(&req, 0, sizeof(Request));
        memset(&res, 0, sizeof(Response));
        int delay_between_retries = 500;
        // 接收客户端的请求
        while(tries < max_retries)
        {
            int ret = recv(client_socket, &req, sizeof(req), 0);
            if( ret < 0)
            {
                LogError("从客户端接收管理员信息失败");
                tries++;
                usleep(delay_between_retries * 10000);
            }
            else if (ret == 0) 
            {
                LogInfo("客户端已断开连接");
                return; // 客户端断开连接，退出循环
            }
            else
            {
                LogInfo("从客户端接收管理员信息成功");
                break;
            }
        }

        switch (req.type)
        {
            case QUERY_WORD:
                {
                    const char *definition = search_word(req.query_word);
                    if(definition)
                    {
                        strncpy(res.definition, definition, sizeof(res.definition) - 1); // 复制查询到的释义到响应
                        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束
                    }
                    else
                    {
                        strncpy(res.definition, "未找到该单词", sizeof(res.definition) - 1);
                        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束
                    }

                    // 保存查询历史
                    pthread_mutex_lock(&global_lock);
                    AddEntryToHistory(db, req.username, req.query_word);
                    pthread_mutex_unlock(&global_lock);

                    // 发送查询结果给客户端
                    tries = 0;

                    while(tries < max_retries)
                    {
                        if(send(client_socket, &res, sizeof(res), 0) >= 0)
                        {
                            LogInfo("发送查询结果给客户端成功");
                            break; // 如果发送成功，跳出循环
                        }
                        else
                        {
                            LogError("发送查询结果给客户端失败");
                            tries++;
                            sleep(1);  // 等待1秒后再次尝试
                        }
                    }
                    if(tries == max_retries)
                    {
                        LogError("尝试发送查询结果给客户端多次失败，放弃发送");
                        return;
                    }
                    break;
                }   
                
            case VIEW_ALL_HISTORY:
                {
                    printf("查询所有历史");

                    char historyBuffer[2048]; // 假设这足够大来容纳所有历史记录
                    memset(historyBuffer, 0, sizeof(historyBuffer));

                    pthread_mutex_lock(&global_lock); // 获取全局数据库锁

                    HistoryBufferContext context = { historyBuffer, sizeof(historyBuffer), 0 };
                    DisplayHistory(db, &context);  // 或者从其他来源获取用户名

                    pthread_mutex_unlock(&global_lock); // 释放全局数据库锁

                    // 填充响应并发送
                    strncpy(res.history, historyBuffer, sizeof(res.history) - 1);
                    res.history[sizeof(res.history) - 1] = '\0'; // 确保字符串结束
                
                    tries = 0;
                
                    while(tries < max_retries)
                    {   
                        if(send(client_socket, &res, sizeof(res), 0) >= 0)
                        {   
                            LogInfo("发送查询历史给客户端成功");
                            break;
                        }
                        else
                        {
                            LogError("发送查询历史给客户端失败");
                            tries++;
                            sleep(1); 
                        }
                    }

                    if(tries == max_retries)
                    {
                        LogError("尝试发送查询历史给客户端多次失败，放弃发送");
                        return;    
                    }

                    break;
                }
            default:
                LogError("无效的管理员请求类型");
                continue_running = false;
                break;
        }
    }
}
// 处理普通用户请求
void handle_user_actions(int client_socket)
{
    Request req; // 用于接收客户端请求
    Response res; // 用于发送给客户端的响应
    int max_retries = 3;
    int tries = 0;
    
    bool continue_running = true;
    while(continue_running)
    {
        memset(&req, 0, sizeof(Request));
        memset(&res, 0, sizeof(Response));

        int ret;
        // 接收客户端的请求
        while(tries < max_retries)
        {
            
            if((ret = recv(client_socket, &req, sizeof(req), 0)) < 0)
            {
                LogError("从客户端接收用户信息失败");
                tries++;
            }
            else if (ret == 0) 
            {
                LogInfo("客户端已断开连接");
                return; // 客户端断开连接，退出循环
            }
            else
            {
                LogInfo("从客户端接收用户信息成功");
                break;
            }
                
        }

        switch (req.type)
        {
            case QUERY_WORD:
                {
                    const char *definition = search_word(req.query_word);
                    if(definition)
                    {
                        strncpy(res.definition, definition, sizeof(res.definition) - 1); // 复制查询到的释义到响应
                        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束
                    }
                    else
                    {
                        strncpy(res.definition, "未找到该单词", sizeof(res.definition) - 1);
                        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束
                    }

                    // 保存查询历史
                    pthread_mutex_lock(&global_lock);
                    AddEntryToHistory(db, req.username, req.query_word);
                    pthread_mutex_unlock(&global_lock);

                    // 发送查询结果给客户端
                    tries = 0;

                    while(tries < max_retries)
                    {
                        if(send(client_socket, &res, sizeof(res), 0) >= 0)
                        {
                            LogInfo("发送查询结果给客户端成功");
                            break; // 如果发送成功，跳出循环
                        }
                        else
                        {
                            LogError("发送查询结果给客户端失败");
                            tries++;
                            sleep(1);  // 等待1秒后再次尝试
                        }
                    }
                    if(tries == max_retries)
                    {
                        LogError("尝试发送查询结果给客户端多次失败，放弃发送");
                        return;
                    }
                    break;
                }   
                
            case VIEW_USER_HISTORY:
                {

                    char historyBuffer[2048]; // 假设这足够大来容纳所有历史记录
                    memset(historyBuffer, 0, sizeof(historyBuffer));

                    pthread_mutex_lock(&global_lock); // 获取全局数据库锁

                    HistoryBufferContext context = { historyBuffer, sizeof(historyBuffer), 0 };
                    DisplayHistoryForUser(db, req.username, &context);

                    pthread_mutex_unlock(&global_lock); // 释放全局数据库锁

                    // 填充响应并发送
                    strncpy(res.history, historyBuffer, sizeof(res.history) - 1);
                    res.history[sizeof(res.history) - 1] = '\0'; // 确保字符串结束
                
                    tries = 0;
                
                    while(tries < max_retries)
                    {   
                        if(send(client_socket, &res, sizeof(res), 0) >= 0)
                        {
                            LogInfo("发送查询历史给客户端成功");
                            break;
                        }
                        else
                        {
                            LogError("发送查询历史给客户端失败");
                            tries++;
                            sleep(1); 
                        }
                    }

                    if(tries == max_retries)
                    {
                        LogError("尝试发送查询历史给客户端多次失败，放弃发送");
                        return;    
                    }

                    break;
                }
            default:
                LogError("无效的用户请求类型");
                continue_running = false;
                break;
        }
    }
}
#endif








