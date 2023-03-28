#include <iostream>
#include <cstring>
#include <unistd.h>    /*unistd.h 是 C 语言标准库中的头文件，通常用于提供一些系统调用相关的函数和符号常量。*/
#include <sys/socket.h>/*sys/socket.h 是 C 语言中提供了一组用于实现网络通信的套接字相关函数和数据类型的头文件。*/
#include <arpa/inet.h> /*<arpa/inet.h> 是 C 语言标准库中提供了一组用于将 IP 地址和网络字节序之间进行转换的函数和数据类型的头文件。*/
#include <pthread.h>   /*pthread.h 是 C 语言中 POSIX 线程库的头文件，该库提供了一组用于创建、同步和管理线程的函数和数据类型。*/
#include <unordered_map>
/*
在 sys/socket.h 头文件中常用的函数包括：

- socket()：创建一个新的套接字。
- bind()：将一个套接字和一个本地地址绑定起来。
- listen()：开始监听一个套接字。
- accept()：接受一个新的客户端连接。
- connect()：向远程主机建立一个连接。
- send()：向一个已经连接的套接字发送数据。
- recv()：从一个已经连接的套接字接收数据。
- close()：关闭一个套接字。
*/

/*
arpa/inet.h头文件定义了以下常用的函数：

inet_addr()：将一个 IP 地址字符串转换为一个网络字节序的整数值。
inet_aton()：将一个 IP 地址字符串转换为一个网络字节序的二进制值。
inet_ntoa()：将一个网络字节序的二进制值转换为一个点分十进制表示的 IP 地址字符串。
htonl() 和 htons()：分别将一个长整型和一个短整型从主机字节序转换到网络字节序。
ntohl() 和 ntohs()：分别将一个长整型和一个短整型从网络字节序转换到主机字节序。

*/

#define MAX_CLIENTS 10                                      // 能接受的最大客户端数量
#define BUFFER_SIZE 1024                                    // 缓冲区长度
pthread_mutex_t clients_mutex_ = PTHREAD_MUTEX_INITIALIZER; /*声明了一个互斥锁变量 clients_mutex，并对其进行初始化。*/
class Server
{
    /*
    使用单例模式来设计这个server服务器
    */
public:
    explicit Server(int port) : port_(port) {} /*构造函数，设置监听端口*/

    // start()为服务器的启动方法
    void start()
    {
        /*
        socket(int domain,int type,int protocol)->success?(A non-negative integer):(-1);
        */
        if ((server_socket_ = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket error");
            exit(EXIT_FAILURE);
        }

        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = INADDR_ANY;
        server_address_.sin_port = htons(port_);

        /*
        int bind(int __fd, const struct sockaddr *__addr, socklen_t __len)
        bind函数告诉内核将 addr 中的服务器套接字地址和套接字描述符 sockfd 联系起来。参数addrlen就是sizeof(sockaddr in)。
        */
        if (bind(server_socket_, reinterpret_cast<struct sockaddr *>(&server_address_), sizeof(server_address_)) == -1)
        {
            perror("bind error");
            exit(EXIT_FAILURE);
        }
        /*
int listen(int __fd, int __n)
服务器调用 listen 函数告诉内核，描述符是被服务器而不是客户端使用的。
*/

        if (listen(server_socket_, MAX_CLIENTS) == -1)
        {
            perror("listen error");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server started. Listening on port " << port_ << "...\n";

        while (true)
        {
            int client_len = sizeof(client_address_);
            // 如果无法accpet，那么报错
            if ((client_socket_ = accept(server_socket_, reinterpret_cast<struct sockaddr *>(&client_address_), reinterpret_cast<socklen_t *>(&client_len))) == -1)
            {
                perror("accept error");
                continue;
            }
            // 如果连接的客户端达到最大值，则拒绝连接
            if (num_clients_ >= MAX_CLIENTS)
            {
                std::cout << "Connection rejected: too many clients\n";
                close(client_socket_);
                continue;
            }
            // 创建线程处理客户端请求
            pthread_t new_thread;
            pthread_create(&new_thread, nullptr, handle_client, &client_socket_);
            pthread_detach(new_thread);
        }
    }

private:
    int server_socket_;
    int client_socket_;
    int port_;
    struct sockaddr_in server_address_
    {
    };
    struct sockaddr_in client_address_
    {
    };
    int clients_[MAX_CLIENTS]; // 记录每个client的socket号
    int num_clients_ = 0;

    static void *handle_client(void *arg)
    {
        int client_socket = *(reinterpret_cast<int *>(arg));
        char buffer[BUFFER_SIZE];

        pthread_mutex_lock(&clients_mutex_);
        Server::instance().clients_[Server::instance().num_clients_++] = client_socket;
        pthread_mutex_unlock(&clients_mutex_);

        std::cout << "Client " << client_socket << " connected.\n";

        while (true)
        {
            memset(buffer, 0, BUFFER_SIZE);

            int recv_res = recv(client_socket, buffer, BUFFER_SIZE, 0);
            printf("client%d:%s", client_socket, buffer);
            if (recv_res <= 0)
            {
                pthread_mutex_lock(&clients_mutex_);
                for (int i = 0; i < Server::instance().num_clients_; i++)
                {
                    if (Server::instance().clients_[i] == client_socket)
                    {
                        Server::instance().num_clients_--;
                        while (i < Server::instance().num_clients_)
                        {
                            Server::instance().clients_[i] = Server::instance().clients_[i + 1];
                            i++;
                        }
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex_);
                std::cout << "Client " << client_socket << " disconnected.\n";
                close(client_socket);
                return nullptr;
            }
            else
            {
                std::string mes;
                mes = "client" + std::to_string((int)client_socket) + std::string(buffer);

                send_message(mes.data(), client_socket);
            }
        }
    }

    // 向除发送人以外的客户端发送信息
    static void send_message(const char *message, int curr)
    {
        pthread_mutex_lock(&clients_mutex_);
        for (int i = 0; i < Server::instance().num_clients_; i++)
        {
            // if (Server::instance().clients_[i] != curr)
            {
                if (send(Server::instance().clients_[i], message, strlen(message), 0) == -1)
                {
                    perror("send error");
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex_);
    }
    // 单例模式
    static Server &instance()
    {
        static Server server(8888);
        return server;
    }
};

int main(int argc, char **argv)
{
    Server server(12345);
    server.start();
    // std::unordered_map<std::string, (void *)()> commands;
    //  处理其他业务
    while (true)
    {
        std::string command;
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command == "quit")
        {
            break;
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }

    return 0;
}