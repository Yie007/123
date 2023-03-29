# 客户端
## 主要功能

客户端可以向服务器发送数据，接收服务器发回的数据。此外还简单模拟了一台打印机用于测试，打印机可以向服务器请求数据并打印，打印机是一直处于打印状态的。

代码中含有些许C和C++混用的情况，敬请原谅！

## 客户端设计

```c++
#define DEFAULT_BUFLEN 1024   // 输入缓冲的最大长度,单位是字节

class Client
{
private:
	//WASDATA是一个结构体，存储初始化Winsock库时的信息
	WSADATA wsaData; 
	//addrinfo是一个结构体，用于设置网络通信的协议、端口号等信息，目的是精确地控制套接字的属性和行为
	struct addrinfo hints; 
	//本机用于连接的套接字
	SOCKET ConnectSocket = INVALID_SOCKET;
	//接收数据的字符数组（输入缓冲）
	char recvBuf[DEFAULT_BUFLEN];
	//输入缓冲的长度
	int recvBufLen = DEFAULT_BUFLEN;
	//接收到的数据的长度
	int recvLen = 0;
public:
	//初始化，返回0表示成功，-1表示出现错误
	int init();

	//根据提供的服务器IP地址以及端口号，连接到服务器，返回0表示成功，-1表示出现错误
	int connectTo(const char* desIP, const char* port);

	//向服务器发送数据，并获取服务器返回的信息，如果要多次发送必须先调用connectTo()，-1表示出现错误，非负整数表示接收到的字节数
	int sendMessage(const char* sendBuf);

	//清除数据
	void close();

	//获取存储在recvBuf内的数据
	string getRecv();
};
```

## 客户端实现

### init()

```C++
int Client::init()
{
	int iResult; //存放状态信息

	//1.初始化Winsock
	// 初始化Winsock，初始化的信息存储在wsaData，如果函数返回非零说明出现错误
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); 
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	//2.规定此次网络通信的一些信息
	// 初始化hint
	ZeroMemory(&hints, sizeof(hints)); 
	// ai_family指定的是IP协议簇，AF_INET（IPv4）、AF_INET6（IPv6）或 AF_UNSPEC（不限制）
	hints.ai_family = AF_UNSPEC; 
	// 指定 socket 的类型，可以是 SOCK_STREAM（流类型）或 SOCK_DGRAM（数据报类型）
	hints.ai_socktype = SOCK_STREAM;  
	// 指定传输协议，可以是 IPPROTO_TCP、IPPROTO_UDP 等
	hints.ai_protocol = IPPROTO_TCP;  
	
	return 0;
}
```

### connectTo()

```c++
int Client::connectTo(const char* desIP, const char* port)
{
	//3.根据传入的要连接的主机的IP地址和端口号，按照hints中制定好的查询结果的期望类型
	//将查询到的地址信息链表头存储在result中，函数返回0表示执行成功
	int iResult;
	struct addrinfo* result = NULL, * ptr = NULL;
	iResult = getaddrinfo(desIP, port, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		//WSACleanup()用于清理和释放 Winsock 库占用的系统资源，并且通知操作系统该线程已经完成网络通信的任务,与WSAStartup()配套使用且一定需要使用一次
		WSACleanup(); 
		return -1;
	}

	// 4.result表示的地址信息链表存储着一组主机（如果有）的信息，只要连接上任意一个即可完成连接任务
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) // ptr就是用来遍历result表示的地址信息链表的
	{
		// 给客户机创建socket
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); 
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError()); // WSAGetLastError()获取错误信息
			WSACleanup();
			return -1;
		}
		// 连接到服务器
		// ai_addr是指向一个sockaddr结构体类型的指针，存放地址信息，这里存放的就是要连接的主机的地址信息
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result); // 释放result链表

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return -1;
	}
	
	return 0;
}
```

### sendMessage()

```c++
int Client::sendMessage(const char* sendBuf)
{
    int iResult;
	//5.发送消息
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}
	iResult = shutdown(ConnectSocket, SD_SEND);
	// 第二个参数表示断开输出流，即告知对方数据已经发送完毕，并且不再发送任何数据
	// 调用 shutdown() 函数会使得套接字进入半关闭状态（half-closed state），即仍然可以接收数据但无法发送数据。
	// 如果需要彻底关闭套接字并释放资源，则需要调用 closesocket() 函数进行关闭。
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}

	//6.接收信息
	// 获取信息直到对方关闭连接
	do
	{
		iResult = recv(ConnectSocket, recvBuf, recvBufLen, 0);
		if (iResult > 0)
		{
			printf("bytes received: %d\n", iResult);
			recvLen = iResult;
		}
		else if (iResult == 0)
		{
			/*printf("connection closed\n");*/
		}
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);

	closesocket(ConnectSocket);
	return iResult;
}
```

### close()

```C++
void Client::close()
{
	WSACleanup();
}
```

### getRecv()

```C++
string Client::getRecv()
{
	string s(recvBuf, recvLen);
	return s;
}
```

## 测试用Demo

最近操作系统课程中学到了多线程的知识（同步、异步、互斥），就想着写测试Demo的时候顺便了解一下。

测试用Demo简单模拟了一台打印机，打印机持续打印处于打印队列中的文件，并且可以并发地向客户端请求数据，加入到打印队列中等待打印，二者都不会阻塞对方。

### 设计

```c++
class Demo
{
private:
	struct CacheData //打印队列的数据单元
	{
		int id;
		string data;
	};

	const int MAX_CACHE_LENGTH = 5; //打印队列的最大长度
	int ID = 1; //全局ID
	queue<CacheData>Q; //打印队列
	mutex m; //锁，用于实现打印队列的互斥访问
	bool flag = true; //标志，用于控制打印机（电源开关）
	condition_variable condInput, condOutput; //条件变量，用于实现线程同步

	//打印处于打印队列中头部位置的文件
	void printData();

	//打印机的工作就是在电源未关闭的情况下持续打印
	void printerAction();

public:
	thread t; //打印线程

	//向服务器发送请求并将获取到的数据压入打印队列中
	void sendRequest(const char* ip, const char* port, const char* request);

	//创建线程
	void printStart();

	//打开电源
	void powerOn();

	//关闭电源
	void powerOff();
};
```

### 实现

```C++
void Demo::sendRequest(const char* ip, const char* port,const char* request)
{
	Client it;
	it.init();
	it.connectTo(ip, port);
	//以这种方式创建锁，在创建的时候自动枷锁==加锁
	unique_lock<mutex>unilock(m);
	//当打印队列已满的时候，停止发送请求
	while (Q.size() > MAX_CACHE_LENGTH)
		condInput.wait(unilock);//等待唤醒，自动调用m.unlock()
	//如果被唤醒，自动调用m.lock()
	it.sendMessage(request);
	it.close();

	CacheData temp;
	temp.id = ID++;
	temp.data = it.getRecv();
	Q.push(temp);//将数据压入打印队列
	condOutput.notify_one();//唤醒打印机，继续打印

	//在退出的时候，自动解锁
	//通常情况下应该是先解锁再唤醒其他线程的
}

void Demo::printData()
{
	//上锁
	unique_lock<mutex>unilock(m);
	while (Q.empty())
		condOutput.wait(unilock);//等待打印队列中出现待打印数据
	CacheData temp = Q.front();
	Q.pop();
	printf("Data %d is printing:  ", temp.id);
	printf("%s\n", temp.data.c_str());
	//为了体现多线程，假设打印需要1秒钟
	unilock.unlock();//手动解锁
	condInput.notify_one();//唤醒发送请求的线程
	Sleep(1000);
}

void Demo::printerAction()
{
	while (flag)
		printData();
}

void Demo::printStart()
{
	//创建线程
	t = thread(&Demo::printerAction, this);
}

void Demo::powerOn()
{
	flag = true;
}

void Demo::powerOff()
{
	flag = false;
}
```

### 测试代码

```C++
int main()
{
	Demo demo;
	printf("|---Demo start.---|\n\n");
	demo.printStart();
	demo.sendRequest("114.132.153.223", "12345", "AAA");
	demo.sendRequest("114.132.153.223", "12345", "BBB");
	demo.sendRequest("114.132.153.223", "12345", "CCC");
	demo.sendRequest("114.132.153.223", "12345", "DDD");
	demo.sendRequest("114.132.153.223", "12345", "EEE");
	demo.sendRequest("114.132.153.223", "12345", "FFF");
	demo.sendRequest("114.132.153.223", "12345", "GGG");
	demo.sendRequest("114.132.153.223", "12345", "HHH");
	demo.sendRequest("114.132.153.223", "12345", "JJJ");
	demo.sendRequest("114.132.153.223", "12345", "KKK");

	if (demo.t.joinable())
		demo.t.join();
	printf("|---Demo end.---|");
}
```

### 测试结果

​	![](.\result.png)

​	显然，在打印阻塞的时候，打印线程依然工作

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

