// echo程序
#include "server.hpp"

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

server_impl::server_impl(int max_client, int buffersize) : MAX_CLIENTS(max_client), BUFFER_SIZE(buffersize)
{
    clients = new int[max_client];
}

bool server_impl::broadCast(char *str)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < this->num_clients; i++)
    {
        if (send_message_to(str, i) == -1)
        {
            perror("send error");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

bool server_impl::send_message_except(char *str, int target)
{
    send_message_except__(str, this->clients[target], *this);
    return true;
}
bool server_impl::send_message_to(char *str, int target)
{
    send_message_to__(str, this->clients[target], *this);
    return true;
}

void *send_message_to__(const char *message, int curr, server_impl inst)
{
    pthread_mutex_lock(&clients_mutex);
    if (send(curr, message, strlen(message), 0) == -1)
    {
        perror("send error");
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *send_message_except__(const char *message, int curr, server_impl inst)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < inst.num_clients; i++)
    {
        if (inst.clients[i] != curr)
        {
            /*
            send(sockfd, buf, len, flags)：向已经连接的套接字 sockfd 发送长度为 len 的数据，数据存储在缓冲区 buf 中，可选的标志位 flags 用于指定发送方式。
            */
            if (send(inst.clients[i], message, strlen(message), 0) == -1)
            {
                perror("send error");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client__(void *info)
{
    handle_client_info argv = *(handle_client_info *)info;
    server_impl *inst = argv.inst;
    int *clients = argv.clients;
    int socketfd = argv.client_socket;
    char buffer[inst->BUFFER_SIZE];
    pthread_mutex_lock(&clients_mutex);
    clients[inst->num_clients++] = socketfd;
    pthread_mutex_unlock(&clients_mutex);

    printf("Client %d connected.\n", socketfd);

    while (1)
    {
        memset(buffer, 0, inst->BUFFER_SIZE);

        /*
        recv(sockfd, buf, len, flags)：从已经连接的套接字 sockfd 接收最多长度为 len 的数据，并将其存储到缓冲区 buf 中，可选的标志位 flags 用于指定接收方式.该函数的返回值是接收到的字节数，如果出现错误则会返回一个负数。
        */
        int recv_res = recv(socketfd, buffer, inst->BUFFER_SIZE, 0);
        if (recv_res <= 0)
        {
            // 如果收到了消息，但是消息却没有东西，或者是收到了错误信息，那么认为这个客户端无法使用，则退出。
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < inst->num_clients; i++)
            {
                if (clients[i] == socketfd)
                {
                    inst->num_clients--;
                    while (i < inst->num_clients)
                    {
                        inst->clients[i] = inst->clients[i + 1];
                        i++;
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            printf("Client %d disconnected.\n", socketfd);
            close(socketfd);
            return NULL;
        }
        else
        {
            std::string mes;
            mes = "client" + std::to_string((int)socketfd) + std::string(buffer) + '\n';
            std::cout << mes;
            send_message_except__(mes.data(), socketfd, *inst);
        }
    }
}

bool server_impl::init()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address; /*ip socket地址的结构*/
    pthread_t new_thread;

    /*
    socket(int domain,int type,int protocol)->success?(A non-negative integer):(-1);
    */
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        return false;
    }

    server_address.sin_family = AF_INET;         // 将地址族（Address Family）设为 IPv4。
    server_address.sin_addr.s_addr = INADDR_ANY; // 将 IP 地址设为 INADDR_ANY，表示可以监听任何可用的本地网络接口。
    server_address.sin_port = htons(12345);      // 将端口号设为 8888，并使用 htons() 函数将其转换为网络字节序

    /*
    int bind(int __fd, const struct sockaddr *__addr, socklen_t __len)
    bind函数告诉内核将 addr 中的服务器套接字地址和套接字描述符 sockfd 联系起来。参数addrlen就是sizeof(sockaddr in)。
    */
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind error");
        return false;
    }

    /*
    int listen(int __fd, int __n)
    服务器调用 listen 函数告诉内核，描述符是被服务器而不是客户端使用的。
    */
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("listen error");
        return false;
    }

    printf("Server started. Listening on port 12345...\n");

    while (1)
    {

        int client_len = sizeof(client_address);
        // 如果无法accpet，那么报错
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_len)) == -1)
        {
            perror("accept error");
            continue;
        }
        // 如果连接的客户端达到最大值，则拒绝连接
        if (num_clients >= MAX_CLIENTS)
        {
            printf("Connection rejected: too many clients\n");
            close(client_socket);
            continue;
        }

        // 创建线程处理客户端请求
        handle_client_info info;
        info.client_socket = client_socket;
        info.clients = this->clients;
        info.inst = this;

        if (pthread_create(&new_thread, NULL, handle_client__, &info) != 0)
        {
            perror("pthread_create error");
            continue;
        }
    }

    return true;
}

int main()
{

    server_impl sv(10, 1024);
    sv.init();
    // optional
    /*
    while(1)
    server.broadCast("Welcome to our rooms!\n");
    server.send_message_to("Pls dont send inappropriate message or you will be kick out!\n", 0);
    server.send_message_except("Those who send inappropriate message will be kick out!\n", 0);
    */
    return 0;
}

/*
```psesudocode

int main(){

    init ...
    server.init();
    do other things.


}


server.init{
    create_thread(){
        listen();
        if(accept()){
            create thread(handle_client());
        }
    }
}





```
*/

/*
```psesudocode
int main(){
    Client client;
    client.init();
    calculate(){
        if(client.read()){
            access
            do sth.
        }else{
            do not access
            do other thing
        }
    }

}









*/

/*
当收到一个客户端信息的时候，转发给其他客户端。
*/

/*
多线程：为每一个客户端单独开辟一条线程
*/