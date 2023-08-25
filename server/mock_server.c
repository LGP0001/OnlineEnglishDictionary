#include "server_handler.h"

#define SERVER_DISCONNECTED -100
volatile bool should_exit = false;

int main(int argc, char const *argv[])
{
    Init_Logger(SERVER_LOG_FILE_PATH);
    int server_socket, client_socket;
    char server_ip[50];
    char server_port[10];
    Addr_in client_addrin;
    socklen_t client_addlen = sizeof(client_addrin);

    if(!load_config("../OnlineEnglishDictionary/config_server.txt", server_ip, server_port))
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
    server_socket = Init_Address(server_ip, server_port, true);
    printf("模拟服务器已在端口 %s 上启动\n", server_port);

    while(1)
    {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addrin, &client_addlen);
        if(client_socket == -1)
        {
            LogError("连接接受失败");
            continue;
        }
        printf("客户端已连接\n");

        handle_login_request(client_socket, db);
    }

    close(client_socket);
    return 0;
}
