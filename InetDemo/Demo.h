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
public:
	struct CacheData
	{
		int id;
		string data;
	};

	const int MAX_CACHE_LENGTH = 3;
	int ID = 1;
	queue<CacheData>Q;
	mutex m;
	condition_variable condInput, condOutput;

	void sendRequest(const char* ip,const char* port,const char* request);

	void printData();

	void printStart()
	{
		int count = 10;
		while (count-->0)
		{
			printData();
		}		
	}

	void sendStart()
	{
		int count = 2;
		while (count--> 0)
		{
			sendRequest("114.132.153.223", "12345", "***");
		}
	}

	Demo()
	{
	}
};
