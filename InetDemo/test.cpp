#include"Client.h"
#include"Demo.h"
#include<iostream>
#include<vector>
using namespace std;
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
