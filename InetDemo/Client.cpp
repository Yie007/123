#include"Client.h"

int Client::init()
{
	int iResult; //存放状态信息

	//1.初始化Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // 初始化Winsock，初始化的信息存储在wsaData，如果函数返回非零说明出现错误
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	//2.规定此次网络通信的一些信息
	ZeroMemory(&hints, sizeof(hints)); // 初始化hint，将内存区域置为0
	hints.ai_family = AF_UNSPEC;       // ai_family指定的是IP协议簇，AF_INET（IPv4）、AF_INET6（IPv6）或 AF_UNSPEC（不限制）
	hints.ai_socktype = SOCK_STREAM;   // 指定 socket 的类型，可以是 SOCK_STREAM（流类型）或 SOCK_DGRAM（数据报类型）
	hints.ai_protocol = IPPROTO_TCP;   // 指定传输协议，可以是 IPPROTO_TCP、IPPROTO_UDP 等
	return 0;

}

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
		WSACleanup(); // 用于清理和释放 Winsock 库占用的系统资源，并且通知操作系统该线程已经完成网络通信的任务,与WSAStartup()配套使用且一定需要使用一次
		return -1;
	}

	// 4.result表示的地址信息链表存储着一组主机（如果有）的信息，只要连接上任意一个即可完成连接任务
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) // ptr就是用来遍历result表示的地址信息链表的
	{
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // 给客户机创建socket
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError()); // WSAGetLastError()获取错误信息
			WSACleanup();
			return -1;
		}

		// 连接到服务器
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);// ai_addr是指向一个sockaddr结构体类型的指针，存放地址信息，这里存放的就是要连接的主机的地址信息
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

int Client::sendMessage(const char* sendBuf)
{
	//5.发送消息

	int iResult;
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
			/*printf("bytes received: %d\n", iResult);*/
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