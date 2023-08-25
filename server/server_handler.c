#include "../server/server_handler.h"

#define SUCCESS 0
#define FAIL -2
extern volatile bool should_exit;
extern pthread_mutex_t global_lock; // 初始化锁

void send_response(int sockfd,  Response *res, int max_retries);
void recv_request(int sockfd, Request *req, int max_retries);
int handle_query_word(int client_socket, Request *req, sqlite3 *db);
int handle_view_all_history(int client_socket, Request *req, sqlite3 *db);
int handle_view_user_history(int client_socket, Request *req, sqlite3 *db);

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        should_exit = true;
        exit(0);
    }
}
// 处理注册请求
void handle_register_request(int client_socket)
{
    Response res; // 响应结构体
    Request req; // 请求结构体
    User new_user;

    memset(&new_user, 0, sizeof(User)); // 初始化用户结构体
    memset(&req, 0, sizeof(Request)); // 初始化请求结构体
    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    res.type = REGISTER;
    strncpy(new_user.username, req.username, sizeof(new_user.username) - 1); // 从请求中复制用户名
    new_user.username[sizeof(new_user.username) - 1] = '\0';  // 确保字符串结束
    strncpy(new_user.password, req.password, sizeof(new_user.password) - 1); // 从请求中复制密码
    new_user.password[sizeof(new_user.password) - 1] = '\0';  // 确保字符串结束

    bool registration_status = register_user(&new_user); 

    if (registration_status)
        res.register_status = true;
    else
        res.register_status = false;

    send_response(client_socket,  &res, 5); // 发送注册结果
}
// 处理登录请求
void handle_login_request(int client_socket, sqlite3 *db, Request req)
{
    Response res; // 响应结构体
    User user;

    memset(&user, 0, sizeof(User)); // 初始化用户结构体
    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    while(!should_exit)
    {  
        int result;
        res.type = LOGIN;           
        strncpy(user.username, req.username, sizeof(user.username) - 1); // 从请求中复制用户名
        user.username[sizeof(user.username) - 1] = '\0';  // 确保字符串结束
        strncpy(user.password, req.password, sizeof(user.password) - 1); // 从请求中复制密码
        user.password[sizeof(user.password) - 1] = '\0';  // 确保字符串结束

        bool is_authenticated = login_user(&user);

        res.login_status = is_authenticated;
        res.is_admin = (strcmp(user.role, "admin") == 0) ? true : false;

        send_response(client_socket,  &res, 5); // 发送登录结果
        if(res.login_status)
        {
            if(res.is_admin)  // 管理员
            {
                printf("[SERVER] 等待管理员请求\n");
                result = handle_admin_actions(client_socket, db);
            }
            else 
            {
                printf("[SERVER] 等待用户请求\n");
                result = handle_user_actions(client_socket, db);
            }
        }
        else
        {
            printf("[SERVER] 登陆失败, 原因: 无此账号\n");
            break;
        }
        if(result ==  -1)
        {
            printf("[SERVER] 处理登录请求失败,请稍后重试\n");
            break;
        }
        if(result == 0)
        {
            printf("[SERVER] 处理登录请求成功\n");
            break;
        }
    }
}
// 处理管理员请求
int handle_admin_actions(int client_socket, sqlite3 *db)
{
    Request req; // 请求结构体
    Response res; // 响应结构体

    memset(&req, 0, sizeof(Request)); // 初始化请求结构体
    memset(&res, 0, sizeof(Response)); // 初始化响应结构体
    
    bool continue_running = true;
    while(continue_running)
    {
        recv_request(client_socket, &req, 5); // 接收管理员操作请求

        int result = 0;
        switch(req.type)
        {
            case QUERY_WORD: // 查询单词请求
                result = handle_query_word(client_socket, &req, db);
                printf("管理员查找单词结果为： %d \n", result);
                break;
            case VIEW_ALL_HISTORY: // 查询所有用户历史请求
                result = handle_view_all_history(client_socket, &req, db); 
                printf("管理员查找历史结果为： %d \n", result);
                break;
            case EXIT:
                res.type = EXIT;
                send_response(client_socket, &res, 5);
                printf("[SERVER] 管理员成功登出");
                continue_running = false;
                result = 1;
                break;
            default:
                LogError("[SERVER] 无效的管理员请求类型");
                break;  
        }
        if(result == FAIL)
        {
            printf("[SERVER] 处理管理员请求失败\n");
            return -1;
        }
        else
        {
            printf("[SERVER] 处理管理员请求成功\n");
        }
    }
    return 0;
}
// 处理普通用户请求
int handle_user_actions(int client_socket, sqlite3 *db)
{
    Request req; // 请求结构体
    Response res; // 响应结构体

    memset(&req, 0, sizeof(Request)); // 初始化请求结构体
    memset(&res, 0, sizeof(Response)); // 初始化响应结构体
    bool continue_running = true;
    while(continue_running)
    {
        recv_request(client_socket, &req, 5); // 接收用户操作请求

        int result;
        switch(req.type)
        {
            case QUERY_WORD: // 查询单词请求
                result = handle_query_word(client_socket, &req, db);
                printf("用户查找单词结果为： %d \n", result);
                break;
            case VIEW_USER_HISTORY: // 查询个人用户历史请求
                result = handle_view_user_history(client_socket, &req, db); 
                printf("用户查找个人历史结果为： %d \n", result);
                break;
            case EXIT:
                res.type = EXIT;
                send_response(client_socket, &res, 5);
                printf("[SERVER] 用户成功登出\n");
                continue_running = false;
                result = 0;
                break;
            default:
                LogError("[SERVER] 无效的个人请求类型\n");
                break;    
        }
        if(result == FAIL)
        {
            printf("[SERVER] 处理普通用户请求失败\n");
            return -1;
        }
        else
        {
            printf("[SERVER] 处理普通用户请求成功\n");
        }
    }
    return 0;
}
// 处理查询单词请求
int handle_query_word(int client_socket, Request *req, sqlite3 *db)
{
    Response res; // 响应结构体

    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    res.type = QUERY_WORD;
    printf("你要查询的单词为: %s \n", req->query_word);   
    const char *definition = search_word(req->query_word);
    if(definition)
    {
        printf("查询到单词释义 \n");
        strncpy(res.definition, definition, sizeof(res.definition) - 1); // 复制查询到的释义到响应
        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束

        // 保存查询历史,后期需要加入锁
        pthread_mutex_lock(&global_lock);
        printf("[SERVER] 正在将查询历史记录 \n");
        AddEntryToHistory(db, req->username, req->query_word);
        printf("[SERVER] 记录完成 \n");
        pthread_mutex_unlock(&global_lock);
    }
    else
    {
        strncpy(res.definition, "未找到该单词", sizeof(res.definition) - 1);
        res.definition[sizeof(res.definition) - 1] = '\0'; // 确保字符串结束

        // 保存查询历史,后期需要加入锁
        pthread_mutex_lock(&global_lock);
        printf("[SERVER] 正在将查询历史记录 \n");
        AddEntryToHistory(db, req->username, req->query_word);
        printf("[SERVER] 记录完成 \n");
        pthread_mutex_unlock(&global_lock);

        return FAIL;
    }
    send_response(client_socket,  &res, 5); // 发送查询结果
    return 0;
}
// 处理查询全部用户历史请求
int handle_view_all_history(int client_socket, Request *req, sqlite3 *db)
{
    Response res; // 响应结构体

    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    res.type = VIEW_ALL_HISTORY;
    char historyBuffer[2048]; // 假设这足够大来容纳所有历史记录
    memset(historyBuffer, 0, sizeof(historyBuffer)); // 初始化缓冲区

    printf("[SERVER] 查询所有历史");
    // 调取查询历史，后期需要加锁

    pthread_mutex_lock(&global_lock);
    HistoryBufferContext context = { historyBuffer, sizeof(historyBuffer), 0 };
    DisplayHistory(db, &context);  // 或者从其他来源获取用户名
    pthread_mutex_unlock(&global_lock);
    // 填充响应并发送
    strncpy(res.history, historyBuffer, sizeof(res.history) - 1);
    res.history[sizeof(res.history) - 1] = '\0'; // 确保字符串结束

    send_response(client_socket, &res, 5);
    return 0;
}
// 处理查询个人用户历史请求
int handle_view_user_history(int client_socket, Request *req, sqlite3 *db)
{
    Response res; // 响应结构体

    memset(&res, 0, sizeof(Response)); // 初始化响应结构体

    res.type = VIEW_USER_HISTORY;
    char historyBuffer[2048]; // 假设这足够大来容纳所有历史记录
    memset(historyBuffer, 0, sizeof(historyBuffer)); // 初始化缓冲区

    printf("[SERVER] 查询%s历史 \n", req->username);
    // 调取查询历史，后期需要加锁
    pthread_mutex_lock(&global_lock);
    HistoryBufferContext context = { historyBuffer, sizeof(historyBuffer), 0 };
    DisplayHistoryForUser(db, req->username, &context);  // 或者从其他来源获取用户名
    pthread_mutex_unlock(&global_lock);
    // 填充响应并发送
    strncpy(res.history, historyBuffer, sizeof(res.history) - 1);
    res.history[sizeof(res.history) - 1] = '\0'; // 确保字符串结束

    send_response(client_socket, &res, 5);
    return 0;
}
void send_response(int sockfd,  Response *res, int max_retries)
{
    int tries = 0;
    while(tries < max_retries)
    {
        int send_bytes = send(sockfd, res, sizeof(*res), 0);

        if(send_bytes == -1)  // 发送请求到服务器
        {
            LogError("向客户端发送信息失败");
            tries++;
            sleep(100);
            continue;
        }
        else
        {
            LogInfo("向客户端发送信息成功");
            printf("已成功发送，发送内容为：\n");
            printf("单词: %s \n", res->word);
            printf("单词解释：%s \n", res->definition);
            printf("登录状态：%s \n", res->login_status ? "true" : "false");
            printf("注册状态：%s \n", res->register_status ? "true" : "false");
            printf("是否为管理员：%s \n", res->is_admin ? "true" : "false");
        }
        break;  // 如果成功发送并接收，跳出循环
    }
    if(tries == max_retries)
    {
        LogError("尝试发送给客户端多次失败，放弃发送");
        return; // 直接从函数返回，不再执行下面的代码
    }
}

void recv_request(int sockfd, Request *req, int max_retries)
{
    int tries = 0;
    while(tries < max_retries)
    {
        int received_bytes = recv(sockfd, req, sizeof(*req), 0); // 接收服务器的响应

        if(received_bytes == -1)
        {
            LogError("从客户端接收信息失败");
            tries++;
            sleep(100);
            continue;
        }
        else if (received_bytes == 0)
        {
            LogInfo("客户端关闭了链接");
            close(sockfd);
        }
        else
        {
            LogInfo("向客户端接收信息成功");
            printf("已接收，接收内容为：\n");
            printf("请求类型: %d\n", req->type);
            printf("用户名为: %s \n", req->username);
            printf("密码为: %s \n", req->password);
            printf("查询单词为: %s \n", req->query_word);
            printf("是否为管理员：%s \n", req->is_admin ? "true" : "false");
        }
        break;
    }
    if(tries == max_retries)
    {
        LogError("尝试从客户端接收信息多次失败，放弃接收");
        return; // 直接从函数返回，不再执行下面的代码
    }
}


