#include"Client.h"

int Client::init()
{
	int iResult; //���״̬��Ϣ

	//1.��ʼ��Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // ��ʼ��Winsock����ʼ������Ϣ�洢��wsaData������������ط���˵�����ִ���
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	//2.�涨�˴�����ͨ�ŵ�һЩ��Ϣ
	ZeroMemory(&hints, sizeof(hints)); // ��ʼ��hint�����ڴ�������Ϊ0
	hints.ai_family = AF_UNSPEC;       // ai_familyָ������IPЭ��أ�AF_INET��IPv4����AF_INET6��IPv6���� AF_UNSPEC�������ƣ�
	hints.ai_socktype = SOCK_STREAM;   // ָ�� socket �����ͣ������� SOCK_STREAM�������ͣ��� SOCK_DGRAM�����ݱ����ͣ�
	hints.ai_protocol = IPPROTO_TCP;   // ָ������Э�飬������ IPPROTO_TCP��IPPROTO_UDP ��
	return 0;

}

int Client::connectTo(const char* desIP, const char* port)
{
	//3.���ݴ����Ҫ���ӵ�������IP��ַ�Ͷ˿ںţ�����hints���ƶ��õĲ�ѯ�������������
	//����ѯ���ĵ�ַ��Ϣ����ͷ�洢��result�У���������0��ʾִ�гɹ�
	int iResult;
	struct addrinfo* result = NULL, * ptr = NULL;
	iResult = getaddrinfo(desIP, port, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup(); // ����������ͷ� Winsock ��ռ�õ�ϵͳ��Դ������֪ͨ����ϵͳ���߳��Ѿ��������ͨ�ŵ�����,��WSAStartup()����ʹ����һ����Ҫʹ��һ��
		return -1;
	}

	// 4.result��ʾ�ĵ�ַ��Ϣ����洢��һ������������У�����Ϣ��ֻҪ����������һ�����������������
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) // ptr������������result��ʾ�ĵ�ַ��Ϣ�����
	{
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // ���ͻ�������socket
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError()); // WSAGetLastError()��ȡ������Ϣ
			WSACleanup();
			return -1;
		}

		// ���ӵ�������
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);// ai_addr��ָ��һ��sockaddr�ṹ�����͵�ָ�룬��ŵ�ַ��Ϣ�������ŵľ���Ҫ���ӵ������ĵ�ַ��Ϣ
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result); // �ͷ�result����

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
	//5.������Ϣ

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
	// �ڶ���������ʾ�Ͽ������������֪�Է������Ѿ�������ϣ����Ҳ��ٷ����κ�����
	// ���� shutdown() ������ʹ���׽��ֽ����ر�״̬��half-closed state��������Ȼ���Խ������ݵ��޷��������ݡ�
	// �����Ҫ���׹ر��׽��ֲ��ͷ���Դ������Ҫ���� closesocket() �������йرա�
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}

	//6.������Ϣ
	// ��ȡ��Ϣֱ���Է��ر�����
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