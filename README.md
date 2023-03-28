# 客户端
#

# 服务端
## 主要功能
接受多个客户端的连接，接收某个客户端的信息，并且转发给其他客户端。
## 设计
``` C++
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
```

