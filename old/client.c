#include "../OnlineEnglishDictionary/utils/common_structs.h"
#include "../OnlineEnglishDictionary/utils/utils.h"
#include "../OnlineEnglishDictionary/logs/logs.h"

void displayLoginMenu() 
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

bool continue_running = true;

int main(int argc, char const *argv[])
{
    // 初始化
    Init_Logger(CLIENT_LOG_FILE_PATH);
    char server_ip[50];
    char server_port[5];
    int choice;

    if(!load_config("./config.txt", server_ip, server_port))
    {
        LogError("加载配置文件错误。\n");
        return -1;
    }

    printf("IP: %s\n", server_ip);
    printf("端口号: %s\n", server_port);

    int sockfd = Init_Address(server_ip, server_port, false);

    // 用户界面
    while(1)
    {
        displayLoginMenu();
        scanf("%d", &choice);
        // 处理用户输入
        Request req;
        Response res;

        switch(choice)
        {
            case 1: // 注册
            {
                req.type = REGISTER;
                printf("请输入您希望的用户名：");
                scanf("%49s", req.username);

                printf("请输入密码: ");
                scanf("%49s", req.password);
                req.is_admin = false; // 默认为普通用户


                int max_retries = 3;
                int tries = 0;

                while(tries < max_retries)
                {
                    if((send(sockfd, &req, sizeof(req), 0)) == -1)  // 发送请求到服务器
                    {
                        LogError("向服务器发送信息失败");
                        tries++;
                        continue;
                    }

                    if((recv(sockfd, &req, sizeof(req), 0)) == -1) // 接收服务器的响应
                    {
                        LogError("从服务器接收信息失败");
                        tries++;
                        continue;
                    }
                    else if ((recv(sockfd, &req, sizeof(req), 0)) == 0)
                    {
                        LogError("服务器关闭了链接");
                        close(sockfd);
                        return -1;
                    }

                    break;  // 如果成功发送并接收，跳出循环
                }

                if(tries == max_retries)
                {
                    LogError("多次尝试与服务器通信失败，请稍后再试。\n");
                    break;
                }
                if (res.register_status) 
                {
                    printf("注册成功！您现在可以登录了。\n");
                } 
                else 
                {
                    printf("注册失败。该用户名可能已被占用。\n");
                }
                break;
            }
            case 2: // 登录
            {
                memset(&req, 0, sizeof(Request));
                memset(&res, 0, sizeof(Response));
                req.type = LOGIN;
                printf("请输入用户名：");
                scanf("%49s", req.username);

                printf("请输入密码: ");
                scanf("%49s", req.password);

                int max_retries = 3;
                int tries = 0;
                bool is_sent = false, is_received = false;

                while(tries < max_retries)
                {
                    if (!is_sent && ((send(sockfd, &req, sizeof(req), 0))) == -1) // 发送请求到服务器 
                    {  
                        LogError("向服务器发送登录请求失败");
                        tries++;
                        continue;
                    }
                    else
                        is_sent = true;

                    int rc = recv(sockfd, &res, sizeof(res), 0);
                    if (!is_received && rc == -1) // 接收服务器的响应
                    {  
                        LogError("从服务器接收登录响应失败");
                        tries++;
                        continue;
                    } 
                    else if (rc == 0) 
                    {
                        LogError("服务器关闭了链接");
                        close(sockfd);
                        return -1;
                    }
                    else
                        is_received = true;

                    if(is_sent && is_received)
                        break;
                }

                if (tries == max_retries)
                {
                    LogError("多次尝试与服务器通信均失败,请稍后重试\n");
                    break;
                }
                    
                if (res.login_status)
                {
                    printf("登录成功！\n");
        
                    bool continue_running = true;

                    if(res.is_admin)  // 进入管理员界面
                    {
                        while (continue_running) 
                        {
                            displayAdminMenu();
                            scanf("%d", &choice);
                            switch (choice)
                            {
                                case 1:
                                {
                                    req.type = QUERY_WORD;
                                    printf("请输入您想查询的单词：");
                                    scanf("%99s", req.query_word);
                                    
                                    if ((send(sockfd, &req, sizeof(req), 0)) == -1) 
                                    {
                                        LogError("向服务器发送查询请求失败");
                                        return -1;
                                    }
                                    else
                                        LogInfo("向服务器发送查询请求成功");
                                    
                                    if ((recv(sockfd, &res, sizeof(res), 0)) == -1) 
                                    {
                                        LogError("从服务器接收查询响应失败");
                                        return -1;
                                    }
                                    else
                                        LogInfo("向服务器接收查询响应成功");

                                    printf("单词释义：%s\n", res.definition);
                                    break;
                                }
                                case 2: // 查看查询历史
                                {
                                    req.type =  VIEW_ALL_HISTORY;
                                    if ((send(sockfd, &req, sizeof(req), 0)) == -1) 
                                    {
                                        LogError("向服务器发送查看历史请求失败");
                                        return -1;
                                    }
                                    else
                                        LogInfo("向服务器发送查看历史请求成功");
                                    if ((recv(sockfd, &res, sizeof(res), 0)) == -1) 
                                    {
                                        LogError("从服务器接收历史记录响应失败");
                                        return -1;
                                    }
                                    else
                                        LogInfo("向服务器接收查看历史请求成功");
                                    printf("查询历史：%s\n", res.history);
                                    break;
                                }
                                case 3: // 退出
                                {
                                    printf("您已顺利登出\n");
                                    continue_running = false;
                                    break;
                                }
                                default:
                                    printf("无效的选项，请重新选择！\n");
                            }
                        }
                    }
                    else // 进入普通用户界面
                    {
                        while (continue_running) 
                        {
                            displayUserMenu();
                            scanf("%d", &choice);
                            switch (choice)
                            {
                                case 1:
                                {
                                    req.type = QUERY_WORD; 
                                    printf("请输入您想查询的单词：");
                                    scanf("%99s", req.query_word);

                                    if ((send(sockfd, &req, sizeof(req), 0)) == -1) 
                                    {
                                        LogError("向服务器发送查询请求失败");
                                        return -1;
                                    }
                                    if ((recv(sockfd, &res, sizeof(res), 0)) == -1) 
                                    {
                                        LogError("从服务器接收查询响应失败");
                                        return -1;
                                    }

                                    printf("单词释义：\n %s\n", res.definition);
                                    break;
                                }
                                case 2: // 查看查询历史
                                {
                                    req.type = VIEW_USER_HISTORY;
                                    if ((send(sockfd, &req, sizeof(req), 0)) == -1) 
                                    {
                                        LogError("向服务器发送查看历史请求失败");
                                        return -1;
                                    }
                                    if ((recv(sockfd, &res, sizeof(res), 0)) == -1) 
                                    {
                                        LogError("从服务器接收历史记录响应失败");
                                        return -1;
                                    }
                                    printf("查询历史：%s\n", res.history);
                                    break;
                                }
                                case 3: // 退出
                                {
                                    printf("您已顺利登出\n");
                                    continue_running = false;
                                    break;
                                }
                                default:
                                    printf("无效的选项，请重新选择！\n");
                            }
                        }
                    }
                }
                else 
                    printf("登录失败。用户名或密码不正确。\n");
                break;
            }

            case 3: // 退出
            {
                printf("感谢您的使用，期待与你的下次相遇\n");
                return 0;
                break;
            }
        }
    }
}




