#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;
#pragma comment(lib, "Ws2_32.lib") // 链接外部库
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512   // 输入缓冲的最大长度,单位是字节

class Client
{
private:
	WSADATA wsaData; //WASDATA是一个结构体，存储初始化Winsock库时的信息
	struct addrinfo hints; //addrinfo是一个结构体，用于设置网络通信的协议、端口号等信息，目的是精确地控制套接字的属性和行为
	SOCKET ConnectSocket = INVALID_SOCKET;
	char recvBuf[DEFAULT_BUFLEN];
	int recvBufLen = DEFAULT_BUFLEN;
	int recvLen = 1;
public:
	//初始化，返回0表示成功，-1表示出现错误
	int init();

	//根据提供的服务器IP地址以及端口号，连接到服务器，返回0表示成功，-1表示出现错误
	int connectTo(const char* desIP, const char* port);

	//向服务器发送数据，并获取服务器返回的信息，如果要多次发送必须先调用connectTo()，-1表示出现错误，非负整数表示接收到的字节数
	int sendMessage(const char* sendBuf);

	//清除
	void close()
	{
		WSACleanup();
	}

	//获取存储在recvBuf内的数据
	string getRecv()
	{
		string s(recvBuf, recvLen);
		return s;
	}
};