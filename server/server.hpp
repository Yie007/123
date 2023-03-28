#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>    /*unistd.h 是 C 语言标准库中的头文件，通常用于提供一些系统调用相关的函数和符号常量。*/
#include <pthread.h>   /*pthread.h 是 C 语言中 POSIX 线程库的头文件，该库提供了一组用于创建、同步和管理线程的函数和数据类型。*/
#include <sys/socket.h>/*sys/socket.h 是 C 语言中提供了一组用于实现网络通信的套接字相关函数和数据类型的头文件。*/
#include <arpa/inet.h> /*<arpa/inet.h> 是 C 语言标准库中提供了一组用于将 IP 地址和网络字节序之间进行转换的函数和数据类型的头文件。*/

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; /*声明了一个互斥锁变量 clients_mutex，并对其进行初始化。*/
class server_impl
{
public:
    const int MAX_CLIENTS; // 能接受的最大客户端数量
    const int BUFFER_SIZE; // 缓冲区长度

    int *clients;
    int num_clients = 0;
    // 构造函数，用于确定最大连接人数和网络缓冲数组的大小
    server_impl(int max_client, int buffersize);
    // init()函数用于初始化网络连接环境并开始监听端口
    bool init();
    // broCast()用于向当前的所有连接用户发送消息
    bool broadCast(char *str);
    // send_message_to()用于向某个特定用户发送消息
    bool send_message_to(char *str, int target);
    // send_message_except()用于向除了这个用户以外所有用户发送消息
    bool send_message_except(char *str, int target);
};

// 内部函数指针，不作为对外接口
void *handle_client__(void *info);
// 内部函数指针，不作为对外接口
void *send_message_to__(char *message, int curr, server_impl inst);
// 内部函数指针，不作为对外接口
void *send_message_except__(char *message, int curr, server_impl inst);

#endif