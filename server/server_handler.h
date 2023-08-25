#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#include "../utils/common_structs.h"
#include "../utils/utils.h"
#include "../logs/logs.h"
#include "../history/history_manager.h"
#include "../dictionary_query/dictionary_query.h"
#include "../user_management/user_management.h"
//#include "../server/concurrent/concurrent.h"
#include <signal.h>
#include <pthread.h>

#define SERVER_DISCONNECTED -100

/*请求操作*/
// 处理注册请求
void handle_register_request(int client_socket);
// 处理登录请求
void handle_login_request(int client_socket, sqlite3 *db, Request req);
// 处理管理员请求
int handle_admin_actions(int client_socket, sqlite3 *db);
// 处理普通用户请求
int handle_user_actions(int client_socket, sqlite3 *db);
// 发送请求
void send_response(int sockfd,  Response *res, int max_retries);

#endif

