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
	thread t;

	//向服务器发送请求并将获取到的数据压入打印队列中
	void sendRequest(const char* ip, const char* port, const char* request);

	//创建线程
	void printStart();

	//打开电源
	void powerOn();

	//关闭电源
	void powerOff();
};
