#include"Demo.h"

void Demo::sendRequest(const char* ip, const char* port,const char* request)
{
	int iResult;
	Client it;
	iResult = it.init();
	if (iResult == -1)
		return;
	iResult = it.connectTo(ip, port);
	if (iResult == -1)
		return;
	//以这种方式创建锁，在创建的时候自动枷锁==加锁
	unique_lock<mutex>unilock(m);
	//当打印队列已满的时候，停止发送请求
	while (Q.size() > MAX_CACHE_LENGTH)
		condInput.wait(unilock);//等待唤醒，自动调用m.unlock()
	//如果被唤醒，自动调用m.lock()
	iResult = it.sendMessage(request);
	if (iResult == -1)
		return;
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

