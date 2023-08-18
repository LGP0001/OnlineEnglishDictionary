#include "utils.h"

/*初始化网络*/
int Init_Address(const char *ip, const char *port, bool server)
{
    int sockfd;
    Addr_in addr_in;
    socklen_t addrlen = sizeof(addr_in);
    // 创建套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        LogError("创建socket失败");
        return ERROR;
    }
    else
    {
        LogInfo("成功创建socket");
    }
    
    bzero(&addr_in, addrlen);
    addr_in.sin_family = AF_INET;
    int parsed_port = atoi(port);// 检查端口
    if(parsed_port <= 0 || parsed_port > 65535)
    {
        LogError("错误端口");
        return ERROR;
    }
    addr_in.sin_port = htons(parsed_port);
    if(inet_aton(ip, &addr_in.sin_addr) == 0)// 地址转换失败
    {
        LogError("错误IP");
        return ERROR;
    }
    else
    {
        LogInfo("成功识别IP");
    }
    if (server) // 判断是否为服务端
    {
        /*地址快速重用*/
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

        /*绑定地址*/
        if(bind(sockfd, (struct sockaddr *)&addr_in, addrlen) < 0)
        {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "无法绑定到IP: %s 和端口: %s. 错误原因: %s", ip, port, strerror(errno));
            LogError(error_msg);
            return ERROR;
        }
        else
        {
            LogInfo("成功绑定地址");
        }
        /*设定为监听模式*/
        if(listen(sockfd, BACKLOG) < 0)
        {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "无法监听到IP: %s 和端口: %s. 错误原因: %s", ip, port, strerror(errno));
            LogError(error_msg);
            return ERROR;
        }
        else
        {
            LogInfo("成功监听地址");
        }
    }
    else /*如果是客户端就发起连接请求*/
    {
        if(connect(sockfd, (struct sockaddr *)&addr_in, addrlen) < 0)
        {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "无法链接到IP: %s 和端口: %s. 错误原因: %s", ip, port, strerror(errno));
            LogError(error_msg);            
            return ERROR;
        }
        else
        {
            LogInfo("成功链接地址");
        }
    }

    return sockfd;
}
