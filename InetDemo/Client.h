#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;
#pragma comment(lib, "Ws2_32.lib") // �����ⲿ��
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512   // ���뻺�����󳤶�,��λ���ֽ�

class Client
{
private:
	WSADATA wsaData; //WASDATA��һ���ṹ�壬�洢��ʼ��Winsock��ʱ����Ϣ
	struct addrinfo hints; //addrinfo��һ���ṹ�壬������������ͨ�ŵ�Э�顢�˿ںŵ���Ϣ��Ŀ���Ǿ�ȷ�ؿ����׽��ֵ����Ժ���Ϊ
	SOCKET ConnectSocket = INVALID_SOCKET;
	char recvBuf[DEFAULT_BUFLEN];
	int recvBufLen = DEFAULT_BUFLEN;
	int recvLen = 1;
public:
	//��ʼ��������0��ʾ�ɹ���-1��ʾ���ִ���
	int init();

	//�����ṩ�ķ�����IP��ַ�Լ��˿ںţ����ӵ�������������0��ʾ�ɹ���-1��ʾ���ִ���
	int connectTo(const char* desIP, const char* port);

	//��������������ݣ�����ȡ���������ص���Ϣ�����Ҫ��η��ͱ����ȵ���connectTo()��-1��ʾ���ִ��󣬷Ǹ�������ʾ���յ����ֽ���
	int sendMessage(const char* sendBuf);

	//���
	void close()
	{
		WSACleanup();
	}

	//��ȡ�洢��recvBuf�ڵ�����
	string getRecv()
	{
		string s(recvBuf, recvLen);
		return s;
	}
};