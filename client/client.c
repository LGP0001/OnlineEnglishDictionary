#include "../utils/common_structs.h"
#include "../utils/utils.h"
#include "../logs/logs.h"

#define SUCCESS 0
#define SERVER_DISCONNECTED -100
#define FAIL -2
void displayLoginMenu(); 
void displayUserMenu();
void displayAdminMenu();
int register_request(int client_socket); // 注册请求
int login_request(int client_socket); // 登录请求
int admin_actions(int client_socket); // 管理员请求
int user_actions(int client_socket); // 普通用户请求
int query_word(int client_socket); // 查单词请求
int view_all_history(int client_socket); // 查找所有用户历史
int view_user_history(int client_socket); // 查找个人历史


int communicate_with_server(int sockfd, Request *req, Response *res, int max_retries);

Request req; // 请求结构体
Response res; // 响应结构体
int main(int argc, char const *argv[])
{
    /*初始化*/
    Init_Logger(CLIENT_LOG_FILE_PATH);
    char client_ip[50];
    char client_port[10];
    int choice = 0;

    if(!load_config("./config_client.txt", client_ip, client_port))
    {
        LogError("加载配置文件错误。\n");
        return -1;
    }

    printf("[CLIENT] IP: %s\n", client_ip);
    printf("[CLIENT] 端口号: %s\n", client_port);

    int client_sockfd = Init_Address(client_ip, client_port, false);

    printf("[CLIENT] socket套接字为: %d\n", client_sockfd);

    bool client_server_connect = true; // 确定循环条件，防止进入死循环

    
    while(client_server_connect)
    {
        displayLoginMenu();
        int ch;
        if(scanf("%d", &choice) != 1) // 如果没有成功读取一个整数
        {
            printf("请输入一个有效的数字！\n");
            while ((ch = getchar()) != '\n' && ch != EOF); // 清空输入缓冲区
            continue; // 跳过此次循环的后续部分，重新显示菜单
        }
        
        while ((ch = getchar()) != '\n' && ch != EOF);
        // 处理用户输入
        int result = 0; // 存储函数的返回值
        switch(choice)
        {
            case 1: // 注册
                result = register_request(client_sockfd);
                break;
            case 2: // 登录
                result = login_request(client_sockfd);
                break;
            case 3: // 退出
                printf("感谢您的使用，期待与你的下次相遇\n");
                req.type = EXIT;
                communicate_with_server(client_sockfd, &req, &res, 5);
                client_server_connect = false;
                close(client_sockfd);
                break;
            default:
                printf("无效的选项，请重新选择！\n");
        }
        if(result == SERVER_DISCONNECTED)
        {
            printf("与服务器的连接已中断。\n");
            client_server_connect = false;
        }
        else if(result == FAIL)
        {
            // 继续执行这个循环。
        }
        else if (result == -1)
        {
            return -1;
        }
        
    }

    return 0;
}

void displayLoginMenu() // 登陆界面
{
    printf("===== 瓜普在线英语词典 =====\n");
    printf("1. 注册\n");
    printf("2. 登录\n");
    printf("3. 退出\n");
    printf("你的选择: ");
}

void displayUserMenu() // 普通用户
{
    printf("===== 瓜普在线英语词典 =====\n");
    printf("1. 查找单词\n");
    printf("2. 我的历史\n");
    printf("3. 登出\n");
    printf("你的选择: ");

}

void displayAdminMenu() // 管理员
{
    printf("===== 瓜普在线英语词典 =====\n");  
    printf("1. 查找单词\n");
    printf("2. 所有用户历史\n");
    printf("3. 登出\n");
    printf("你的选择: ");
}

int register_request(int client_socket) // 注册请求
{
    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    req.type = REGISTER; // 声明为注册

    printf("请输入您希望的用户名：");
    scanf("%49s", req.username);

    printf("请输入密码: ");
    scanf("%49s", req.password);
    req.is_admin = false; // 默认为普通用户

    int comm_status;
    do
    {
        printf("请确认信息：\n");
        printf("用户名为:%s \n", req.username);
        printf("密码为:%s \n", req.password);
        printf("确认请按Enter \n");

        char ch = getchar();
        while ((ch != '\n') && ch != EOF) 
            ch = getchar();  // 继续读取直到遇到换行符或文件结束符

        comm_status = communicate_with_server(client_socket, &req, &res, 5);
        if(comm_status == SERVER_DISCONNECTED)
            return SERVER_DISCONNECTED;
    } while (comm_status == -1);
    
    if(res.register_status)
    {
        printf("注册成功！您现在可以登录了。\n");
        return SUCCESS;
    }
    else
    {
        printf("注册失败。请另选一个用户名。\n");
        return FAIL;
    }
}

int login_request(int client_socket) // 登录请求
{
    Request req; // 请求结构体
    Response res; // 响应结构体

    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    req.type = LOGIN; // 声明为登录

    printf("请输入用户名：");
    scanf("%49s", req.username);

    printf("请输入密码: ");
    scanf("%49s", req.password);

    int comm_status;
    do
    {
        printf("请确认信息：\n");
        printf("用户名为:%s \n", req.username);
        printf("密码为:%s \n", req.password);
        printf("确认请按Enter \n");

        char ch = getchar();
        while ((ch != '\n') && ch != EOF) 
            ch = getchar();  // 继续读取直到遇到换行符或文件结束符

        comm_status = communicate_with_server(client_socket, &req, &res, 5);
        if(comm_status == SERVER_DISCONNECTED)
            return SERVER_DISCONNECTED;
    } while (comm_status == -1);
    
    if(res.login_status)
    {
        printf("登录成功！欢迎回来。\n");
        int action_result;
        if(res.is_admin)
            action_result = admin_actions(client_socket);
        else
            action_result = user_actions(client_socket);

        if(action_result == SUCCESS)
        {
            printf("操作成功！\n");
            return SUCCESS; 
        }
        else if(action_result == -1)
        {
            printf("操作失败。\n");
            return -1; 
        }
        else
        {
            printf("服务器中断。\n");
            return -1; 
        }
    }
    else
    {
        printf("登录失败。用户名或密码不正确。\n");
        return -1;
    }
}

int admin_actions(int client_socket) // 管理员请求
{
    bool continue_running = true;

    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    while(continue_running)
    {
        displayAdminMenu();
        int choice;
        int ch;
        if(scanf("%d", &choice) != 1) // 如果没有成功读取一个整数
        {
            printf("请输入一个有效的数字！\n");
            while ((ch = getchar()) != '\n' && ch != EOF); // 清空输入缓冲区
            continue; // 跳过此次循环的后续部分，重新显示菜单
        }
        
        while ((ch = getchar()) != '\n' && ch != EOF);
        // 处理用户输入
        int result = 0; // 存储函数的返回值
        switch(choice)
        {
            case 1: // 查单词
                result = query_word(client_socket);
                break;
            case 2: // 查询全部用户历史
                result = view_all_history(client_socket);
                break;
            case 3: // 登出
            {
                req.type = EXIT;
                communicate_with_server(client_socket, &req, &res, 5);
                printf("您已顺利登出\n");
                continue_running = false;
                break;
            }
            default:
                printf("无效的选项，请重新选择！\n");

        }
        if(result == SERVER_DISCONNECTED)
        {
            printf("与服务器的连接已断开。\n");
            continue_running = false;
            return SERVER_DISCONNECTED;
        }
        else if (result == -1)
        {
            return -1;
        }
        
    }
    return 0;
}

int user_actions(int client_socket) // 普通用户请求
{
    bool continue_running = true;

    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    while(continue_running)
    {
        displayUserMenu();
        int choice;
        int ch;
        if(scanf("%d", &choice) != 1) // 如果没有成功读取一个整数
        {
            printf("请输入一个有效的数字！\n");
            while ((ch = getchar()) != '\n' && ch != EOF); // 清空输入缓冲区
            continue; // 跳过此次循环的后续部分，重新显示菜单
        }
        
        while ((ch = getchar()) != '\n' && ch != EOF);
        // 处理用户输入
        int result = 0; // 存储函数的返回值
        switch(choice)
        {
            case 1: // 查单词
                result = query_word(client_socket);
                break;
            case 2: // 查询全部用户历史
                result = view_user_history(client_socket);
                break;
            case 3: // 登出
            {
                printf("您已顺利登出\n");
                req.type = EXIT;
                communicate_with_server(client_socket, &req, &res, 5);
                continue_running = false;
                result = 0;
                break;
            }
            default:
                printf("无效的选项，请重新选择！\n");
                break;

        }
        if(result == SERVER_DISCONNECTED)
        {
            printf("与服务器的连接已断开。\n");
            continue_running = false;
            return SERVER_DISCONNECTED;
        }
        else if (result == -1)
        {
            printf("操作失败");
            return -1;
        }
    }
    return 0;
}

int query_word(int client_socket) // 查单词请求
{
    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));
    
    req.type = QUERY_WORD;

    printf("请输入您想查询的单词：");
    scanf("%99s", req.query_word);

    int status = communicate_with_server(client_socket, &req, &res, 5);
    if(status != 0)
        return status;
    
    printf("单词释义：%s \n", res.definition);

    return 0;                               
}

int view_all_history(int client_socket) // 查找所有用户历史
{
    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    req.type =  VIEW_ALL_HISTORY;
    int status = communicate_with_server(client_socket, &req, &res, 5);
    if(status != 0)
        return status;

    printf("查询历史：%s\n ", res.history);
    return 0;
}
int view_user_history(int client_socket) // 查找个人历史
{
    memset(&req, 0, sizeof(Request));
    memset(&res, 0, sizeof(Response));

    req.type =  VIEW_USER_HISTORY;
    int status = communicate_with_server(client_socket, &req, &res, 5);
    if(status != 0)
        return status;

    printf("查询历史：%s\n ", res.history);
    return 0;
}

int communicate_with_server(int sockfd, Request *req, Response *res, int max_retries) // 与服务器的通信
{
    int tries = 0;

    while(tries < max_retries)
    {
        int send_bytes = send(sockfd, req, sizeof(*req), 0);

        if(send_bytes == -1)  // 发送请求到服务器
        {
            LogError("向服务器发送信息失败");
            tries++;
            sleep(100);
            continue;
        }
        else
        {
            LogInfo("向服务器发送信息成功");
            printf("已成功发送，发送内容为：\n");
            printf("请求类型: %d\n", req->type);
            printf("用户名为: %s \n", req->username);
            printf("密码为: %s \n", req->password);
            printf("查询单词为: %s \n", req->query_word);
            printf("是否为管理员：%s \n", req->is_admin ? "true" : "false");
        }

        int received_bytes = recv(sockfd, res, sizeof(*res), 0); // 接收服务器的响应

        if(received_bytes == -1)
        {
            LogError("从服务器接收信息失败");
            tries++;
            sleep(100);
            continue;
        }
        else if (received_bytes == 0)
        {
            LogError("服务器关闭了链接");
            close(sockfd);
            return SERVER_DISCONNECTED;
        }
        else
        {
            LogInfo("向服务器接收信息成功");
            printf("已接收，接收内容为：\n");
            printf("接收类型: %d\n", res->type);
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
        LogError("多次尝试与服务器通信失败，请稍后再试。\n");
        return -1;
    }

    return 0;
}




