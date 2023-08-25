#include "../server/thread.h"

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER; // 初始化全局锁
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER; // 初始化日志锁
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER; // 条件变量，通常与互斥锁配合使用来进行线程间同步
// static FILE *logFile = NULL;
volatile bool should_exit = false; // 用于指示线程何时应退出
extern sqlite3 *db; // 全局数据库变量
pthread_t threads[MAX_THREADS]; // 存储线程标识符的数组

int client_queue[MAX_CLIENTS] = {0}; // 客户端队列，用于存储待处理的客户端
int queue_front = 0, queue_rear = -1, queue_count = 0; // 线程持队列头，尾和队列元素个数
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER; // 初始化客户端队列的互斥锁

/*线程操作*/

// 初始化并发处理模块
void initialize_concurrent_module(int num_threads)
{
    if(num_threads > MAX_THREADS)
    {
        pthread_mutex_lock(&log_lock);
        LogError("[THREAD] 超过最大线程数");
        pthread_mutex_unlock(&log_lock);
        return; // 如果超出线程限制，直接返回        
    }
    // 创建线程
    for(int i = 0; i < num_threads; i++)
    {
        int threads_num = i + 1;
        if(pthread_create(&threads[i], NULL, client_handler_thread, (void*)(size_t)threads_num) != 0)
        {
            pthread_mutex_lock(&log_lock);
            LogError("线程创建失败");
            pthread_mutex_unlock(&log_lock);
            return; // 如果线程创建失败，终止进程
        }
    }
    LogInfo("并发处理模块初始化完成");
}
// 客户端处理线程函数
void *client_handler_thread()
{
    while (!should_exit)
    {
        int client_socket = dequeue_client();
        printf("取到线程中的套接字是: %d\n", client_socket);
        if(client_socket < -1) // 队列为空，线程暂停一段时间，然后再次尝试
        {
            LogInfo("队列已空");
            sleep(10);
            continue;
        }
        else
        {
            LogInfo("队列中有嵌套字");
            printf("嵌套字为： %d\n", client_socket);
        }

        Request req; // 请求结构体
        Response res; // 响应结构体

        memset(&req, 0, sizeof(Request)); // 初始化请求结构体
        memset(&res, 0, sizeof(Response)); // 初始化响应结构体

        int read_size; 
        bool should_continue = true;     
        do
        {
            printf("正在等待接收\n");
            read_size = recv(client_socket, &req, sizeof(Request), 0);
            printf("已经接收到客户端请求\n");
            printf("接收字节 %d\n", read_size);
            printf("请求类型: %d\n", req.type);
            printf("用户名为: %s \n", req.username);
            printf("密码为: %s \n", req.password);
            printf("查询单词为: %s \n", req.query_word);
            printf("是否为管理员：%s \n", req.is_admin ? "true" : "false");
            
            
            if(read_size > 0)
            {
                switch (req.type)
                {
                    case REGISTER:
                    {
                        handle_register_request(client_socket);
                        break;
                    }
                    case LOGIN:
                    {
                        handle_login_request(client_socket, db, req);
                        break;
                    }
                    case EXIT:
                    {
                        res.type = EXIT;
                        send_response(client_socket, &res, 5);
                        pthread_mutex_lock(&log_lock);
                        LogInfo("客户端断开连接");
                        pthread_mutex_unlock(&log_lock);
                        remove_client(client_socket);
                        close(client_socket);
                        should_continue = false;
                        break;
                    }
                    default:
                        LogError("未知的请求类型");
                        break;
                }
            }
            else if(read_size == 0)
            {
                pthread_mutex_lock(&log_lock);
                LogInfo("客户端已中断");
                pthread_mutex_unlock(&log_lock);
                close(client_socket);
                remove_client(client_socket);
                should_continue = false; 
                break;  // 当客户端断开连接时，退出循环
            }
            else if (read_size == -1)
            {
                pthread_mutex_lock(&log_lock);
                LogError("接收错误");
                perror("recv error"); 
                pthread_mutex_unlock(&log_lock);
                should_continue = false; 
                break;  // 当接收错误时，退出循环
            }
        }
        while (read_size > 0 && should_continue);  
    }

    return NULL;
}
// 添加客户端队列元素
void enqueue_client(int client_socket)
{
    pthread_mutex_lock(&queue_lock); // 获取队列的互斥锁，确保在此时没有其他线程修改队列

    if(queue_count == MAX_CLIENTS) // 队列满了
    {
        LogError("队列满了");
        close(client_socket); // 关闭client_socket，避免资源泄露
    }
    else
    {
        queue_rear = (queue_rear + 1) % MAX_CLIENTS;     // 先移动指针
        client_queue[queue_rear] = client_socket;        // 后添加

        queue_count++;
    }
        
    pthread_cond_signal(&queue_cond); // 通知至少一个正在等待条件变量的线程，条件已经发生
    pthread_mutex_unlock(&queue_lock);
   

    printf("添加队列元素后队列中元素为: %d\n", queue_count);
    for (int i = queue_front; i != queue_rear; i = (i + 1) % MAX_CLIENTS)
    {
        printf("每个队列元素为 %d\n", client_queue[i]);
    }
}
// 取出客户端队列元素
int dequeue_client(void)
{
    pthread_mutex_lock(&queue_lock); // 获取队列的互斥锁，确保在此时没有其他线程修改队列
    while(queue_count <= 0 && !should_exit) // 当队列为空并且不需要退出时，等待其他线程发出条件变量信号，这通常发生在一个新的客户端socket被加入队列之后
    {
        pthread_cond_wait(&queue_cond, &queue_lock); // 释放互斥锁并等待条件变量信号，当信号到来时，重新获取互斥锁并继续执行
    }
    if (should_exit) // 如果退出，则释放互斥锁并返回-1，表示无客户端socket可用
    {
        pthread_mutex_unlock(&queue_lock);
        return -1;
    }
    
    printf("取出队列元素前队列前元素个数为: %d\n", queue_count);
    int client_socket = client_queue[queue_front]; // 从队列前端取出一个客户端socket
    queue_front = (queue_front + 1) % MAX_CLIENTS; // 更新队列前端的索引（循环队列）
    queue_count--; // 更新队列中的元素数量

    pthread_mutex_unlock(&queue_lock); // 释放互斥锁
    printf("取出队列元素后队列中元素个数为: %d\n", queue_count);
    printf("取出的队列元素为: %d\n", client_socket);
    for (int i = queue_front; i < queue_count; i++)
    {
        printf("每个队列元素为为 %d\n", client_queue[i]);
    }
    return client_socket; // 返回取出的客户端socket
}
// 移除一个已经处理完的客户端队列元素
void remove_client(int client_socket)
{    
    pthread_mutex_lock(&queue_lock); // 获取队列的互斥锁，确保在此时没有其他线程修改队列
    
    int i;
    int found = -1;
    for(i = queue_front; i != queue_rear; i = (i + 1) % MAX_CLIENTS) // 遍历队列，查找与指定的client_socket匹配的元素
    {
        if(client_queue[i] == client_socket) 
        {
            found = i;
            break;
        }
    }
    
    if(found != -1) // 如果找到了匹配的元素，开始移除它
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
}