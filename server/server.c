#include "../server/server_handler.h"
#include "../server/thread.h"

#define SERVER_DISCONNECTED -100
extern volatile bool should_exit;

int main(int argc, char const *argv[])
{

    int server_socket, client_socket;
    char server_ip[50];
    char server_port[10];
    // 初始化日志
    Init_Logger(SERVER_LOG_FILE_PATH);
    // 加载网络配置
    if(!load_config("../OnlineEnglishDictionary/config_server.txt", server_ip, server_port))
    {
        LogError("加载网络配置文件错误。\n");
        return -1;
    }
    else
        LogInfo("加载网络配置文件成功。\n");
    // 加载词典数据
    if (!load_dictionary("../OnlineEnglishDictionary/dictionary_query/dict.txt")) 
    {
        LogError("词典加载失败");
        return -1;
    }
    else
        LogInfo("加载词典成功。\n");
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
    int num_threads = 5; // 启动的线程数量
    initialize_concurrent_module(num_threads);
    // 初始化服务器
    server_socket = Init_Address(server_ip, server_port, true);
    if (server_socket == ERROR) 
    {
        LogError("服务器初始化失败");
        return -1;  // 退出程序
    }
    LogInfo("等待链接");
    printf("[SERVER] 服务器已在端口 %s 上启动\n", server_port);
    printf("[SERVER] socket套接字为: %d\n", server_socket);

    while(!should_exit)
    {
        Addr_in client_addr_in;
        socklen_t client_addr_len = sizeof(client_addr_in);

        client_socket = accept(server_socket, (struct sockaddr*)&client_addr_in, &client_addr_len);
        if(client_socket == -1)
        {
            LogError("连接接受失败");
            continue;
        }
        printf("客户端已连接, 客户端套接字为: %d\n", client_socket);
        // 添加到客户端队列
        enqueue_client(client_socket);
    }
    // 清理资源
    shutdown_server(num_threads);
    ClearHistory(db); 
    Close_Database(db);
    close(server_socket);
    close(client_socket);
    return 0;
}