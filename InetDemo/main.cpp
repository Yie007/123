#include"Client.h"
#include"Demo.h"
#include<iostream>
#include<vector>
using namespace std;
int main()
{
	Demo demo;
	printf("|---Demo start.---|\n\n");
	vector<thread>thr;
	for (int i = 0; i < 5; ++i)
	{
		thr.push_back(thread([&demo]() {demo.sendStart(); }));
	}
	thread t(thread([&demo]() {demo.printStart(); }));
	
	for (int i = 0; i < 10; ++i) {
		Sleep(1000);
		if (thr[i].joinable())
			thr[i].join();
	}
		if (t.joinable())
			t.join();
	
	
	printf("|---Demo end.---|");

	//Client me;
	//me.init();
	//me.connectTo("114.132.153.223", "12345");
	//me.sendMessage("111");
	//cout<<me.getRecv();
	//me.close();

}
