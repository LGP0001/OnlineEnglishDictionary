#include "utils.h"

/*初始化网络*/
int Init_Address(const char *ip, const char *port, bool server)
{
    int sockfd;
    Addr_in addr_in;
    socklen_t addrlen = sizeof(addr_in);
    // 创建套接字
    if(sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0)
    {
            fprintf(stderr, "创建socket失败\n");
            return ERROR;
    }
    bzero(&addr_in, addrlen);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(atoi(port));
    if(inet_aton(ip, &addr_in.sin_addr) == 0)// 地址转换失败
    {
        fprintf(stderr, "错误IP\n");
        return ERROR;
    }
    if (server) // 判断是否为服务端
    {
        /*地址快速重用*/
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

        /*绑定地址*/
        if(bind(sockfd, (struct sockaddr *)&addr_in, addrlen) < 0)
        {

            fprintf(stderr, "无法绑定到IP: %s 和端口: %s. 错误原因: %s\n", ip, port, strerror(errno));
            return ERROR;
        }
        /*设定为监听模式*/
        if(listen(sockfd, BACKLOG) < 0)
        {
            fprintf(stderr, "无法监听到IP: %s 和端口: %s. 错误原因: %s\n", ip, port, strerror(errno));
            return ERROR;
        }
    }
    else /*如果是客户端就发起连接请求*/
    {
        if(connect(sockfd, (struct sockaddr *)&addr_in, addrlen) < 0)
        {
            fprintf(stderr, "无法链接到IP: %s 和端口: %s. 错误原因: %s\n", ip, port, strerror(errno));
            return ERROR;
        }
    }

    return sockfd;
}
