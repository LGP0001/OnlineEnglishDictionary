#include "../OnlineEnglishDictionary/utils/common_structs.h"
#include "../OnlineEnglishDictionary/utils/utils.h"
#include "../OnlineEnglishDictionary/logs/logs.h"
#include "../OnlineEnglishDictionary/history/history_manager.h"
#include "../OnlineEnglishDictionary/dictionary_query/dictionary_query.h"
#include "../OnlineEnglishDictionary/user_management/user_management.h"
#include "../OnlineEnglishDictionary/server/concurrent/concurrent.h"
#include <signal.h>

extern volatile bool should_exit;

void handle_signal(int signal);

int main(int argc, const char *argv[]) 
{
    int server_sockfd, client_sockfd;
    char server_ip[50];
    char server_port[10];

    signal(SIGINT, handle_signal);
    // 初始化日志
    Init_Logger(SERVER_LOG_FILE_PATH);
    // 加载网络配置
    if(!load_config("./config.txt", server_ip, server_port))
    {
        LogError("加载网络配置文件错误。\n");
        return -1;
    }
    // 加载词典数据
    if (!load_dictionary("../OnlineEnglishDictionary/dictionary_query/dict.txt")) 
    {
        LogError("词典加载失败");
        return -1;
    }
    // 初始化数据库
    sqlite3 *db = Init_Database("../OnlineEnglishDictionary/test.db");
    if (!db) 
    {
        LogError("数据库初始化失败");
        return -1;
    }
    // 创建查询历史表
    CreateHistoryTable(db);
    setup_user_database(db);
    // 初始化并发模块
    int num_threads = 5; // 例如，这可以是你想要启动的线程数量
    initialize_concurrent_module(num_threads);
    // 初始化网络
    server_sockfd = Init_Address(server_ip, server_port, true);
    if (server_sockfd == ERROR) 
    {
        LogError("服务器初始化失败");
        return -1;  // 退出程序
    }
    LogInfo("等待链接");

    while (!should_exit) 
    {
        Addr_in client_addr_in;
        socklen_t client_addr_len = sizeof(client_addr_in);

        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr_in, &client_addr_len);
        if (client_sockfd < 0) 
        {
            LogError("客户端接受失败");
            continue;  // 跳过错误，继续下一个循环
        }

        // 添加到客户端队列
        enqueue_client(client_sockfd);
    }
    // 清理资源
    shutdown_server(num_threads);
    ClearHistory(db); // 如果需要的话
    Close_Database(db);
    close(server_sockfd);
    return 0;
}

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        should_exit = true;
        exit(0);
    }
}