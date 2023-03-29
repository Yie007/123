#pragma once
#include "Client.h"
#include<thread>
#include<string>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<Windows.h>
#include<iostream>

using namespace std;

class Demo
{
private:
	struct CacheData //��ӡ���е����ݵ�Ԫ
	{
		int id;
		string data;
	};

	const int MAX_CACHE_LENGTH = 5; //��ӡ���е���󳤶�
	int ID = 1; //ȫ��ID
	queue<CacheData>Q; //��ӡ����
	mutex m; //��������ʵ�ִ�ӡ���еĻ������
	bool flag = true; //��־�����ڿ��ƴ�ӡ������Դ���أ�
	condition_variable condInput, condOutput; //��������������ʵ���߳�ͬ��

	//��ӡ���ڴ�ӡ������ͷ��λ�õ��ļ�
	void printData();

	//��ӡ���Ĺ��������ڵ�Դδ�رյ�����³�����ӡ
	void printerAction();

public:
	thread t;

	//��������������󲢽���ȡ��������ѹ���ӡ������
	void sendRequest(const char* ip, const char* port, const char* request);

	//�����߳�
	void printStart();

	//�򿪵�Դ
	void powerOn();

	//�رյ�Դ
	void powerOff();
};
